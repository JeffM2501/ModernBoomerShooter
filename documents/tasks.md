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

## Add Music system
Add a system to play music tracks and change them based on triggers

## Add MOB Respawner
API to trigger respawn based on conditions or triggers

## Add Map transtion Triggers
Component to move from one map to another on trigger

## Add timed trigger component
Component that fires an event after an entity has been in the trigger for some specified time.

## Interactable triggers
Triggers that fire event when an action key is pressed and user is in area.

# In Progress
--------------------------------------------------------------
## Add MOB system
Mob components and mob processing system, seperate from map object system

# Commplete
--------------------------------------------------------------
## Add global event listeners
Add callbacks that can be triggered fron events from any object. Have object fire events to global system.

## Draw top and bottom of doors
Doors need tops and bottoms now that they can be vertical.