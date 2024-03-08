/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define HST_LOG_TAG "AVOutputFilter"
#include "av_transport_output_filter.h"
#include "av_trans_log.h"
#include "av_trans_constants.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/plugin_attr_desc.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AVOutputFilter"

static AutoRegisterFilter<AVOutputFilter> g_registerFilterHelper("builtin.avtransport.avoutput");

AVOutputFilter::AVOutputFilter(const std::string& name) : FilterBase(name), plugin_(nullptr), pluginInfo_(nullptr)
{
    AVTRANS_LOGI("ctor called");
}

AVOutputFilter::~AVOutputFilter()
{
    AVTRANS_LOGI("dtor called");
}

std::vector<WorkMode> AVOutputFilter::GetWorkModes()
{
    return {WorkMode::PUSH};
}

ErrorCode AVOutputFilter::SetParameter(int32_t key, const Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        AVTRANS_LOGE("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (plugin_ != nullptr) {
        plugin_->SetParameter(static_cast<Plugin::Tag>(key), value);
    }
    {
        std::lock_guard<std::mutex> lock(paramsMapMutex_);
        paramsMap_[tag] = value;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::GetParameter(int32_t key, Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        AVTRANS_LOGE("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    {
        std::lock_guard<std::mutex> lock(paramsMapMutex_);
        value = paramsMap_[tag];
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Prepare()
{
    AVTRANS_LOGI("Prepare entered.");
    if (state_ != FilterState::INITIALIZED) {
        AVTRANS_LOGE("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    state_ = FilterState::PREPARING;
    ErrorCode err = FindPlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Find plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = InitPlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Init plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = ConfigPlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Configure downStream fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = PreparePlugin();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Prepare plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    state_ = FilterState::READY;
    AVTRANS_LOGI("Prepare end.");
    return err;
}

ErrorCode AVOutputFilter::Start()
{
    AVTRANS_LOGI("Start");
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        AVTRANS_LOGE("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Start()) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("The plugin start fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Stop()
{
    AVTRANS_LOGI("Stop");
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (state_ != FilterState::RUNNING) {
        AVTRANS_LOGE("The current state is invalid");
        plugin_->Deinit();
        return ErrorCode::SUCCESS;
    }
    if (TranslatePluginStatus(plugin_->Stop()) != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("The plugin stop fail!");
    }
    plugin_->Deinit();
    plugin_ = nullptr;
    state_ = FilterState::READY;
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Pause()
{
    AVTRANS_LOGI("Pause");
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Resume()
{
    AVTRANS_LOGI("Resume");
    return ErrorCode::SUCCESS;
}

void AVOutputFilter::InitPorts()
{
    AVTRANS_LOGI("InitPorts");
    auto inPort = std::make_shared<InPort>(this);
    {
        std::lock_guard<std::mutex> lock(outputFilterMutex_);
        inPorts_.push_back(inPort);
    }
}

ErrorCode AVOutputFilter::FindPlugin()
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    std::string mime;
    if (paramsMap_.find(Tag::MIME) == paramsMap_.end() ||
        !Plugin::Any::IsSameTypeWith<std::string>(paramsMap_[Tag::MIME])) {
        AVTRANS_LOGE("Must set mime correctly first");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    mime = Plugin::AnyCast<std::string>(paramsMap_[Tag::MIME]);
    auto nameList = PluginManager::Instance().ListPlugins(PluginType::GENERIC_PLUGIN);
    for (const std::string& name : nameList) {
        auto info = PluginManager::Instance().GetPluginInfo(PluginType::GENERIC_PLUGIN, name);
        if (info->inCaps.empty() || mime != info->inCaps[0].mime) {
            continue;
        }
        if (CreatePlugin(info) == ErrorCode::SUCCESS) {
            AVTRANS_LOGI("CreatePlugin %{public}s success", name_.c_str());
            return ErrorCode::SUCCESS;
        }
    }
    AVTRANS_LOGI("Cannot find any plugin");
    return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
}

bool AVOutputFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Capability>& upstreamCap,
    Plugin::Capability& negotiatedCap, const Plugin::Meta& upstreamParams, Plugin::Meta& downstreamParams)
{
    AVTRANS_LOGI("Negotiate");
    if (pluginInfo_ == nullptr) {
        AVTRANS_LOGE("pluginInfo_ is nullptr");
        return false;
    }
    negotiatedCap = pluginInfo_->inCaps[0];
    return true;
}

ErrorCode AVOutputFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& selectedInfo)
{
    AVTRANS_LOGI("CreatePlugin");
    if (selectedInfo == nullptr || selectedInfo->name.empty()) {
        AVTRANS_LOGE("selectedInfo is nullptr or pluginName is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (selectedInfo->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            AVTRANS_LOGI("Reuse last plugin: %{public}s", selectedInfo->name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            AVTRANS_LOGE("Deinit last plugin: %{public}s error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>(selectedInfo->name);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("PluginManager CreatePlugin %{public}s fail", selectedInfo->name.c_str());
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    pluginInfo_ = selectedInfo;
    AVTRANS_LOGI("Create new plugin: %{public}s success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}

bool AVOutputFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta,
    Plugin::Meta& upstreamParams, Plugin::Meta& downstreamParams)
{
    AVTRANS_LOGI("DoConfigure");
    return true;
}

ErrorCode AVOutputFilter::InitPlugin()
{
    AVTRANS_LOGI("InitPlugin");
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    return TranslatePluginStatus(plugin_->Init());
}

ErrorCode AVOutputFilter::ConfigPlugin()
{
    AVTRANS_LOGI("Configure");
    ErrorCode err = SetPluginParams();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Set Plugin fail!");
        return err;
    }
    err = SetEventCallBack();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Plugin SetEventCallBack fail!");
        return err;
    }
    err = SetDataCallBack();
    if (err != ErrorCode::SUCCESS) {
        AVTRANS_LOGE("Plugin SetDataCallBack fail!");
        return err;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::PreparePlugin()
{
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    return TranslatePluginStatus(plugin_->Prepare());
}

ErrorCode AVOutputFilter::PushData(const std::string& inPort, const AVBufferPtr& buffer, int64_t offset)
{
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (buffer == nullptr || plugin_ == nullptr) {
        AVTRANS_LOGE("buffer or plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    plugin_->PushData(inPort, buffer, offset);
    AVTRANS_LOGI("push buffer to plugin.");
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::SetPluginParams()
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (paramsMap_.find(Tag::MEDIA_DESCRIPTION) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::MEDIA_DESCRIPTION, paramsMap_[Tag::MEDIA_DESCRIPTION]);
    }
    if (paramsMap_.find(Tag::AUDIO_CHANNELS) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::AUDIO_CHANNELS, paramsMap_[Tag::AUDIO_CHANNELS]);
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_RATE) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::AUDIO_SAMPLE_RATE, paramsMap_[Tag::AUDIO_SAMPLE_RATE]);
    }
    if (paramsMap_.find(Tag::AUDIO_CHANNEL_LAYOUT) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, paramsMap_[Tag::AUDIO_CHANNEL_LAYOUT]);
    }
    if (paramsMap_.find(Tag::SECTION_USER_SPECIFIC_START) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::SECTION_USER_SPECIFIC_START, paramsMap_[Tag::SECTION_USER_SPECIFIC_START]);
    }
    if (paramsMap_.find(Tag::SECTION_VIDEO_SPECIFIC_START) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::SECTION_VIDEO_SPECIFIC_START, paramsMap_[Tag::SECTION_VIDEO_SPECIFIC_START]);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::SetEventCallBack()
{
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    plugin_->SetCallback(this);
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::SetDataCallBack()
{
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        AVTRANS_LOGE("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    plugin_->SetDataCallback(std::bind(&AVOutputFilter::OnDataCallback, this, std::placeholders::_1));
    return ErrorCode::SUCCESS;
}

void AVOutputFilter::OnDataCallback(std::shared_ptr<Plugin::Buffer> buffer)
{
    std::lock_guard<std::mutex> lock(outputFilterMutex_);
    if (buffer == nullptr) {
        AVTRANS_LOGE("buffer is nullptr!");
        return;
    }
    OnEvent(Event{name_, EventType::EVENT_BUFFER_PROGRESS, buffer});
}
} // namespace DistributedHardware
} // namespace OHOS