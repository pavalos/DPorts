--- src/plugins/positionprovider/gpsd/GpsdPositionProviderPlugin.cpp.orig	2013-03-01 06:49:42.245424000 +0000
+++ src/plugins/positionprovider/gpsd/GpsdPositionProviderPlugin.cpp
@@ -12,7 +12,7 @@
 
 #include "GpsdThread.h"
 #include "MarbleDebug.h"
-#include <math.h>
+#include <cmath>
 
 using namespace Marble;
 /* TRANSLATOR Marble::GpsdPositionProviderPlugin */
@@ -76,7 +76,7 @@ void GpsdPositionProviderPlugin::update(
 {
     PositionProviderStatus oldStatus = m_status;
     GeoDataCoordinates oldPosition = m_position;
-    if ( data.status == STATUS_NO_FIX || isnan( data.fix.longitude ) || isnan( data.fix.latitude ) )
+    if ( data.status == STATUS_NO_FIX || std::isnan( data.fix.longitude ) || std::isnan( data.fix.latitude ) )
         m_status = PositionProviderStatusUnavailable;
     else {
         m_status = PositionProviderStatusAvailable;
@@ -88,29 +88,29 @@ void GpsdPositionProviderPlugin::update(
 
         m_accuracy.level = GeoDataAccuracy::Detailed;
 #if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 3 )
-        if ( !isnan( data.fix.epx ) && !isnan( data.fix.epy ) ) {
+        if ( !std::isnan( data.fix.epx ) && !std::isnan( data.fix.epy ) ) {
             m_accuracy.horizontal = qMax( data.fix.epx, data.fix.epy );
         }
 #else
-        if ( !isnan( data.fix.eph ) ) {
+        if ( !std::isnan( data.fix.eph ) ) {
             m_accuracy.horizontal = data.fix.eph;
         }
 #endif
-        if ( !isnan( data.fix.epv ) ) {
+        if ( !std::isnan( data.fix.epv ) ) {
             m_accuracy.vertical = data.fix.epv;
         }
 
-        if( !isnan(data.fix.speed ) )
+        if( !std::isnan(data.fix.speed ) )
         {
             m_speed = data.fix.speed;
         }
 
-        if( !isnan( data.fix.track ) )
+        if( !std::isnan( data.fix.track ) )
         {
             m_track = data.fix.track;
         }
 
-        if ( !isnan( data.fix.time ) )
+        if ( !std::isnan( data.fix.time ) )
         {
             m_timestamp = QDateTime::fromMSecsSinceEpoch( data.fix.time * 1000 );
         }
