////////////////////////////////////////////////////////////
//
// DAGON - An Adventure Game Engine
// Copyright (c) 2011 Senscape s.r.l.
// All rights reserved.
//
// NOTICE: Senscape permits you to use, modify, and
// distribute this file in accordance with the terms of the
// license agreement accompanying it.
//
////////////////////////////////////////////////////////////

#ifndef DG_SCENE_H
#define DG_SCENE_H

////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////

class DGCameraManager;
class DGConfig;
class DGCursorManager;
class DGRenderManager;
class DGRoom;
class DGTexture;

////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////

class DGScene {
    // References to singletons
    DGCameraManager* cameraManager;    
    DGConfig* config;
    DGCursorManager* cursorManager;   
    DGRenderManager* renderManager;
    
    // Other classes
    DGRoom* _currentRoom;
    DGTexture* _splashTexture;
    
    bool _canDrawSpots; // This bool is used to make checks faster
    bool _isSplashLoaded;
    
public:
    DGScene();
    ~DGScene();
    
    void clear();
    void drawSpots();
    void fadeIn();    
    void fadeOut(); 
    bool scanSpots();
    void setRoom(DGRoom* room);
    
    // Splash screen operations
    void drawSplash();
    void loadSplash();
    void unloadSplash();
};

#endif // DG_SCENE_H