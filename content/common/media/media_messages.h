// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, hence no include guard here, but see below
// for a much smaller-than-usual include guard section.

#include <string>
#include <vector>

#include "base/memory/shared_memory.h"
#include "content/common/content_export.h"
#include "content/common/content_param_traits.h"
#include "content/common/media/media_process_launch_causes.h"
#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT

#define IPC_MESSAGE_START MediaMsgStart

IPC_ENUM_TRAITS_MAX_VALUE(content::CauseForMediaLaunch,
                          content::CAUSE_FOR_MEDIA_LAUNCH_MAX_ENUM - 1)

//------------------------------------------------------------------------------
// Media Messages
// These are messages from the browser to the media process.

// Tells the media process to initialize itself. The browser explicitly
// requests this be done so that we are guaranteed that the channel is set
// up between the browser and media process before doing any work that might
// potentially crash the media process. Detection of the child process
// exiting abruptly is predicated on having the IPC channel set up.
IPC_MESSAGE_CONTROL0(MediaMsg_Initialize)

// Tells the media process to create a new channel for communication with a
// given client.  The channel name is returned in a
// MediaHostMsg_ChannelEstablished message.  The client ID is passed so that
// the media process reuses an existing channel to that process if it exists.
// This ID is a unique opaque identifier generated by the browser process.
IPC_MESSAGE_CONTROL1(MediaMsg_EstablishChannel, int /* client_id */)

// Tells the media process to close the channel identified by IPC channel
// handle.  If no channel can be identified, do nothing.
IPC_MESSAGE_CONTROL1(MediaMsg_CloseChannel,
                     IPC::ChannelHandle /* channel_handle */)

// Tells the media process to clean all resources.
IPC_MESSAGE_CONTROL0(MediaMsg_Clean)

// Tells the media process to crash.
IPC_MESSAGE_CONTROL0(MediaMsg_Crash)

// Tells the media process to hang.
IPC_MESSAGE_CONTROL0(MediaMsg_Hang)

//------------------------------------------------------------------------------
// Media Host Messages
// These are messages to the browser.

// A renderer sends this when it wants to create a connection to the media
// process. The browser will create the media process if necessary, and will
// return a handle to the channel via a MediaChannelEstablished message.
IPC_SYNC_MESSAGE_CONTROL1_2(MediaHostMsg_EstablishMediaChannel,
                            content::CauseForMediaLaunch,
                            int /* client id */,
                            IPC::ChannelHandle /* handle to channel */)

// Response from the media process to a MediaHostMsg_Initialize message.
IPC_MESSAGE_CONTROL1(MediaHostMsg_Initialized, bool /* result */)

// Response from the media process to a MediaHostMsg_EstablishChannel message.
IPC_MESSAGE_CONTROL1(MediaHostMsg_ChannelEstablished,
                     IPC::ChannelHandle /* channel_handle */)

// Message from the media process to notify to destroy the channel.
IPC_MESSAGE_CONTROL1(MediaHostMsg_DestroyChannel, int32 /* client_id */)

// Message from the media process to add a media process log message to the
// about:media page.
IPC_MESSAGE_CONTROL3(MediaHostMsg_OnLogMessage,
                     int /*severity*/,
                     std::string /* header */,
                     std::string /* message */)
