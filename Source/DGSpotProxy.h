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

#ifndef DG_SPOTPROXY_H
#define DG_SPOTPROXY_H

////////////////////////////////////////////////////////////
// NOTE: This header file should never be included directly.
// It's auto-included by DGProxy.h
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "DGAudioManager.h"
#include "DGSpot.h"
#include "DGTexture.h"
#include "DGVideoManager.h"

////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////

class DGSpotProxy : public DGObjectProxy {
public:
    static const char className[];
    static Luna<DGSpotProxy>::RegType methods[];
    
    // Constructor
    DGSpotProxy(lua_State *L) {
        int flags = DGSpotUser;
        float volume = 1.0f;
        
        int params = lua_gettop(L);
        int direction = kNorth;
        int coordsIndex;
        int flagsIndex;
        
        if (params == 3) {
            direction = luaL_checknumber(L, 1);
            coordsIndex = 2;
            flagsIndex = 3;
        }
        else if (params == 2) {
            if (lua_isnumber(L, 1)) {
                direction = lua_tonumber(L, 1);
                coordsIndex = 2;
                flagsIndex = 3; // Won't pass the check
            }
            else {
                coordsIndex = 1;
                flagsIndex = 2;
            }
        }
        else if (params == 1) {
			flagsIndex = 3; // Won't pass the check
            coordsIndex = 1;
        }
        else return; // Everything is wrong! Do panic!
        
        if (lua_istable(L, flagsIndex)) {
            lua_pushnil(L);
            while (lua_next(L, flagsIndex) != 0) {
                const char* key = lua_tostring(L, -2);
                
                // We can only read the key as a string, so we have no choice but
                // do an ugly nesting of strcmps()
                if (strcmp(key, "auto") == 0) {
                    if (lua_toboolean(L, -1) == 1) flags = flags | DGSpotAuto;
                    else flags = flags & ~DGSpotAuto;
                }
                
                if (strcmp(key, "loop") == 0) {
                    if (lua_toboolean(L, -1) == 1) flags = flags | DGSpotLoop;
                    else flags = flags & ~DGSpotLoop;
                }
                
                if (strcmp(key, "sync") == 0) {
                    if (lua_toboolean(L, -1) == 1) flags = flags | DGSpotSync;
                    else flags = flags & DGSpotSync;
                }

                if (strcmp(key, "volume") == 0) volume = (float)(lua_tonumber(L, -1) / 100);
                
                lua_pop(L, 1);
            }
        }
        
        if (lua_istable(L, coordsIndex)) {
            std::vector<int> arrayOfCoords;
            
            lua_pushnil(L);  // First key
            while (lua_next(L, coordsIndex) != 0) {
                // Uses key at index -2 and value at index -1
                arrayOfCoords.push_back(lua_tonumber(L, -1));
                lua_pop(L, 1);
            }
    
            s = new DGSpot(arrayOfCoords, direction, flags);
            s->setVolume(volume);
        }
        else luaL_error(L, kString14007);
        
        // Init the base class
        this->setObject(s);
    }
    
    // TODO: In the future we should return a pointer to an attached object
    int attach(lua_State *L) {
        Action action;
        Audio* audio;
        DGTexture* texture;
        DGVideo* video;
        
        // For the video attach, autoplay defaults to true
        bool autoplay, loop, sync;
        
        int type = (int)luaL_checknumber(L, 1);
        
        switch (type) {
            case AUDIO:
                if (DGCheckProxy(L, 2) == DGObjectAudio) {
                    // Just set the audio object
                    s->setAudio(DGProxyToAudio(L, 2));
                    
                    // Now we get the metatable of the added audio and set it
                    // as a return value
                    lua_getfield(L, LUA_REGISTRYINDEX, AudioProxyName);
                    lua_setmetatable(L, -2);
                    
                    return 1;
                }
                else {
                    // If not, create and set (deleted by the Audio Manager)
                    audio = new Audio;
                    audio->setResource(luaL_checkstring(L, 2));
                    
                    DGAudioManager::instance().registerAudio(audio);
                    
                    if (s->hasFlag(DGSpotLoop))
                        audio->setLoopable(true);
                  
                    s->setAudio(audio);
                }
                
                break;
            case FEED:
                action.type = kActionFeed;
                action.cursor = kCursorLook;
                action.luaObject = s->luaObject();                
                action.feed = luaL_checkstring(L, 2);
            
                if (lua_isstring(L, 3)) {
                    action.feedAudio = lua_tostring(L, 3);
                    action.hasFeedAudio = true;
                }
                else action.hasFeedAudio = false;
                
                s->setAction(&action);
                if (!s->hasColor())
                    s->setColor(0);
                
                break;
            case FUNCTION:
                if (!lua_isfunction(L, 2)) {
                    Log::instance().error(kModScript, "%s", kString14009);
                    return 0;
                }
                
                action.type = kActionFunction;
                action.cursor = kCursorUse;
                action.luaObject = s->luaObject();
                action.luaHandler = luaL_ref(L, LUA_REGISTRYINDEX); // Pop and return a reference to the table
                
                s->setAction(&action);
                if (!s->hasColor())
                    s->setColor(0);
                
                break;                
            case IMAGE:
                // FIXME: This texture isn't managed. We should be able to communicate with the
                // texture manager, register this texture, and parse its path if necessary, OR
                // use the DGControl register method.
                
                // TODO: Decide here if we have an extension and therefore set the name or the
                // resource of the texture.
                
                texture = new DGTexture;
                texture->setResource(Config::instance().path(kPathResources, luaL_checkstring(L, 2), DGObjectImage).c_str());
                
                // If we have a third parameter, use it to set the index inside a bundle
                if (lua_isnumber(L, 3))
                    texture->setIndexInBundle(lua_tonumber(L, 3));
                
                s->setTexture(texture);
                break;
            case SWITCH:
                action.type = kActionSwitch;
                action.cursor = kCursorForward;
                action.luaObject = s->luaObject();
                
                type = DGCheckProxy(L, 2);
                if (type == DGObjectNode)
                    action.target = DGProxyToNode(L, 2);
                else if (type == DGObjectSlide) {
                    action.cursor = kCursorLook;
                    action.target = DGProxyToSlide(L, 2);
                }
                else if (type == DGObjectRoom)
                    action.target = DGProxyToRoom(L, 2);
                else {
                    Log::instance().error(kModScript, "%s", kString14008);
                    return 0;
                }
                
                s->setAction(&action);
                if (!s->hasColor())
                    s->setColor(0);
                
                break;
            case VIDEO:
                if (s->hasFlag(DGSpotAuto)) autoplay = true;
                else autoplay = false;
                
                if (s->hasFlag(DGSpotLoop)) loop = true;
                else loop = false;
                
                if (s->hasFlag(DGSpotSync)) sync = true;
                else sync = false;                
                    
                video = new DGVideo(autoplay, loop, sync);
                
                // TODO: Path is set by the video manager
                video->setResource(Config::instance().path(kPathResources, luaL_checkstring(L, 2), DGObjectVideo).c_str());
                s->setVideo(video);
                DGVideoManager::instance().registerVideo(video);
                
                break;                
        }

        return 0;
    }
    
    // Set a cursor for the attached action
    int setCursor(lua_State *L) {
        if (s->hasAction()) {
            Action* action = s->action();
            
            action->cursor = luaL_checknumber(L, 1);
        }
        
        return 0;
    }
    
    // Check if playing
    int isPlaying(lua_State *L) {
        lua_pushboolean(L, s->isPlaying());
        return 1;
    }    
    
    // Play the spot
    int play(lua_State *L) {
        // WARNING: Potential thread conflict here! Let the manager decide when to pause
        // thread(s).
        
        s->play();
        
        // Test for synced audio or video
        if (s->hasVideo()) {
            if (s->isPlaying() && s->video()->isSynced()) {
                DGControl::instance().syncSpot(s);
                return DGScript::instance().suspend();
            }
        }
        
        return 0;
    }
    
    // Stop the spot
    int stop(lua_State *L) {
        s->stop();
        return 0;
    }
    
    // Destructor
    ~DGSpotProxy() { delete s; }
    
    DGSpot* ptr() { return s; }
    
private:
    DGSpot* s;  
};

////////////////////////////////////////////////////////////
// Static definitions
////////////////////////////////////////////////////////////

const char DGSpotProxy::className[] = DGSpotProxyName;

Luna<DGSpotProxy>::RegType DGSpotProxy::methods[] = {
    DGObjectMethods(DGSpotProxy),    
    method(DGSpotProxy, attach),
    method(DGSpotProxy, setCursor),    
    method(DGSpotProxy, isPlaying),    
    method(DGSpotProxy, play),
    method(DGSpotProxy, stop),   
    {0,0}
};

#endif // DG_SPOTPROXY_H
