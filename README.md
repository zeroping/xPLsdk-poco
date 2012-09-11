xPLsdk-poco
===========

A version of the C++ xPL SDK, converted to run on top of the cross-platform POCO libraries.

The original comes from Mal Lansell, located at http://www.xplmonkey.com/xplsdk.html. This version was based on version 4.3.0, released 21st January 2007.

At this point, there are some substansial changes. Some parts don't work yet (like saving configuration). The events and threads used previously have been replaced with POCO notifications, which all execute in the thread that sends the notification.

For an example, try the included ConsoleApp.cpp.
