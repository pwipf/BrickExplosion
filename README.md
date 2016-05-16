# Virtual World

## Summary

Well, the idea is to just explore a bit.  The controls are wasd and you can jump with the space but there is nothing really to jump over.
Mouse moves view with left button pushed.  Left button can also activate the action when close to an (the) object and standing still.
  
There are some settings in the ui, mainly there are a lot of vertices in the bricks and my computer runs it smoothly at detail lvl 4 and struggles with 5, and another computer might need detail turned down more to do the animation.  
  
There are some walls and you have to pretend you can't walk through them.  
  
There is an object inside that you can activate by mouse clicking when you are close to it and standing still.  Then you can just watch.  It's fun to go see the action from outside at some point.  

After the action you can go activate the object again to reset everything but this time with different lighting and no more activation.  
  
This is late as usual and at the moment it is crashing on mac, and I have no idea why.  
   Ok working on mac but no floor or roof, doesn't like the second texture.
  
   OK, for mac just have to find the line mac=0 at beginning of glwidget.cpp glwidget constructor.  Change to 1 which disables rendering the wood texture objects so it doesn't crash. 
  
Hopefully is still worth some  points a couple days late.  

Features:  
  
- Cube mapped sky  
- Instanced bricks (and mortar) which worked well to animate them... I think I could call this a particle system :)  
- Lots of lighting effects that are barely if at all used.  

--------------------------------




