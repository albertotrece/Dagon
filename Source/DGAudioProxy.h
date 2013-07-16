////////////////////////////////////////////////////////////
//
// DAGON - An Adventure Game Engine
// Copyright (c) 2011-2013 Senscape s.r.l.
// All rights reserved.
//
// This Source Code Form is subject to the terms of the
// Mozilla Public License, v. 2.0. If a copy of the MPL was
// not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
//
////////////////////////////////////////////////////////////

#ifndef DG_AUDIOPROXY_H
#define DG_AUDIOPROXY_H

////////////////////////////////////////////////////////////
// NOTE: This header file should never be included directly.
// It's auto-included by DGProxy.h
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "DGAudioManager.h"

////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////

class DGAudioProxy : public DGObjectProxy {
public:
    static const char className[];
    static Luna<DGAudioProxy>::RegType methods[];
    
    // Constructor
    DGAudioProxy(lua_State *L) {
        a = new DGAudio;
        
        a->setResource(luaL_checkstring(L, 1));

        if (lua_istable(L, 2)) {
            lua_pushnil(L);
            while (lua_next(L, 2) != 0) {
                const char* key = lua_tostring(L, -2);

                if (strcmp(key, "loop") == 0) a->setLoopable(lua_toboolean(L, -1));
                if (strcmp(key, "volume") == 0) a->setDefaultFadeLevel((float)(lua_tonumber(L, -1) / 100));
                
                lua_pop(L, 1);
            }
        }
        
        // Register the new audio
        DGAudioManager::getInstance().registerAudio(a);
        
        // Init the base class
        this->setObject(a);
    }
    
    // Destructor
    ~DGAudioProxy() { delete a; }
    
    // Match with another audio
    int match(lua_State *L) {
        a->match((DGAudio*)DGProxyToAudio(L, 1));
        
        return 0;
    } 
    
    // Pause the audio
    int pause(lua_State *L) {
        a->pause();
        
        return 0;
    } 
    
    // Play the audio
    int play(lua_State *L) {
        // Request the audio
        DGAudioManager::getInstance().requestAudio(a);
        a->play();
        
        return 0;
    }
    
    // Stop the audio
    int stop(lua_State *L) {
        a->stop();
        
        return 0;
    } 
    
    DGAudio* ptr() { return a; }
    
private:
    DGAudio* a;
    
};

////////////////////////////////////////////////////////////
// Static definitions
////////////////////////////////////////////////////////////

const char DGAudioProxy::className[] = DGAudioProxyName;

Luna<DGAudioProxy>::RegType DGAudioProxy::methods[] = {
    DGObjectMethods(DGAudioProxy),
    method(DGAudioProxy, match),
    method(DGAudioProxy, play),
    method(DGAudioProxy, pause),    
    method(DGAudioProxy, stop),
    {0,0}
};

#endif // DG_AUDIOPROXY_H
