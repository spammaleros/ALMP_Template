//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////
// AsioSourceSinkFactory.h
/// \file
/// Plug-in unique ID and creation functions (hooks) necessary to register the mixer plug-in in the sound engine.
/// <br><b>Wwise effect name:</b>  Asio Input/Output
/// <br><b>Library file:</b> AkAsio.lib
#pragma once

#ifdef AK_WIN

#include <AK/SoundEngine/Common/IAkPlugin.h>

AK_STATIC_LINK_PLUGIN(ASIOSource)
AK_STATIC_LINK_PLUGIN(ASIOSink)

#endif
