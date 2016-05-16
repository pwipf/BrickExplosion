# Virtual World
![image](/cover.jpg?raw=true "cover")  

![image](/house.jpg?raw=true "house")  

![image](/houseFire.jpg?raw=true "houseFire")  

![image](/brickExplosion1.jpg?raw=true "brickExplosion1")  

![image](/brickExplosion2.jpg?raw=true "brickExplosion2")  

![image](/fallingBricks.jpg?raw=true "fallingBricks")  

![image](/gimbal.jpg?raw=true "gimbal")  



## Summary

This is a Qt 5 program, [www.qt.io](https://www.qt.io/download-open-source/), written for a graphics course for learning openGL.  The code can easily be built and run with the QtCreator IDE that comes with Qt 5, or built with qmake and make, or mingw32-make, etc.  The simplest way to see the program work is probably to open the .pro file in QtCreator and run from there.  

The program is not really optimized or meant to be a full-fledged game or anything like that, instead it is more an exploration of basic openGL concepts, with various geometry generation, texture application, lighting models, and animation effects thrown in.  

At the moment the code is very unorganized but I will continue to refine and commment it to make it easier to follow, in case someone wants to learn from it.

## Controls
Mouse moves view with left button pushed.  Typical "wasd" first person controls, space to jump.  

Pretend you can't walk through the walls.  

Activate the object by clicking it when you are close enough, then you can just move around and watch the effects.  The idea is that the gimbal thing is some sort of power source that drains energy from around it then goes faster and faster eventually causing an explosion.
  
There are some settings in the ui, mainly there are a lot of vertices in the bricks and my computer runs it smoothly at detail lvl 4 and struggles with 5, and another computer might need detail turned down more to do the animation.  


Issues:  
  
- I mixed something up with setting the active texture so that on a Mac it crashes.  For now, to run on a Mac find the line that says mac=0 at beginning of glwidget.cpp and change to 1.  This disables rendering the roof and floor but it doesn't crash. 

--------------------------------




