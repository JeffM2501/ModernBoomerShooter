# ToDo
--------------------------------------------------------------
## Add assert system
Add assert macro that hits an assert system, on assert take a screenshot get some state. Have the main loop check for asserts and if there is an assert state draw the assert instead of the game.

## Add API to set door state
Allow other systems to force a door open or closed

## Add door security
Add a basic door security system that is extensiable via scripting.

## Add split doors
Add option for split doors, vertical and horizontal.

## Add start state for doors.
Let doors start open or closed.

## Add max/min state for doors.
Let some doors not open all the way.

## Add stay open state for doors
Add an option to allow doors to stay open when triggered.

## Only draw meshes in visable cells
Map each mesh's bbox to the vis cell list.

## Sort cells far to near
Bucket the vis list by the distance the line has walked so that it can be sorted far to near for transperancy.

## Transperant cells
Add option to give a cell a transpeerant wall and floor.

## Only draw meshes in visable cells
Map each mesh's bbox to the vis cell list.

## Sort cells far to near
Bucket the vis list by the distance the line has walked so that it can be sorted far to near for transperancy.

## Transperant cells
Add option to give a cell a transpeerant wall and floor.

## Add Billobard system
Compute billboard characters and faces from relative rotation/facing

## Add MOB system
Mob components and mob processing system, seperate from map object system

## Support rotation/flip of walls and floors
Allow faces to be rotated or flipped based on tmx flags?


# In Progress
--------------------------------------------------------------

## Fix world lighting to use lighting shader
Use the lighting shadder for ambient, but still apply AO and lighting zones.

# Commplete
--------------------------------------------------------------
## Add global event listeners
Add callbacks that can be triggered fron events from any object. Have object fire events to global system.

## Draw top and bottom of doors
Doors need tops and bottoms now that they can be vertical.