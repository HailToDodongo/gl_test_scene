Libdragon patch (besides the usual removal of ogl code):<br/>
Reduces DMEM usage for perf. counters:
```patch
diff --git a/include/rspq_constants.h b/include/rspq_constants.h
index 5f532af9..4230bd08 100644
--- a/include/rspq_constants.h
+++ b/include/rspq_constants.h
@@ -1,8 +1,8 @@
 #ifndef __RSPQ_INTERNAL
 #define __RSPQ_INTERNAL
 
-#define RSPQ_DEBUG                     1
-#define RSPQ_PROFILE                   0
+#define RSPQ_DEBUG                     0
+#define RSPQ_PROFILE                   1
 
 #define RSPQ_DRAM_LOWPRI_BUFFER_SIZE   0x200   ///< Size of each RSPQ RDRAM buffer for lowpri queue (in 32-bit words)
 #define RSPQ_DRAM_HIGHPRI_BUFFER_SIZE  0x80    ///< Size of each RSPQ RDRAM buffer for highpri queue (in 32-bit words)
@@ -67,13 +67,15 @@
 /** Debug marker in DMEM to check that C and Assembly have the same DMEM layout */
 #define RSPQ_DEBUG_MARKER            0xABCD0123
 
+#define RSPQ_PROFILE_OVERLAY_COUNT      4 // RSPQ_MAX_OVERLAY_COUNT
+
 #define RSPQ_PROFILE_SLOT_SIZE          8
-#define RSPQ_PROFILE_SLOT_COUNT         (RSPQ_MAX_OVERLAY_COUNT + 6)
+#define RSPQ_PROFILE_SLOT_COUNT         (RSPQ_PROFILE_OVERLAY_COUNT + 6)
 #define RSPQ_PROFILE_BUILTIN_SLOT       0
-#define RSPQ_PROFILE_IDLE_SLOT          RSPQ_MAX_OVERLAY_COUNT
-#define RSPQ_PROFILE_IDLE_RDP_SLOT      (RSPQ_MAX_OVERLAY_COUNT + 1)
-#define RSPQ_PROFILE_IDLE_SYNC_SLOT     (RSPQ_MAX_OVERLAY_COUNT + 2)
-#define RSPQ_PROFILE_RDPQ_SYNC_SLOT     (RSPQ_MAX_OVERLAY_COUNT + 3)
-#define RSPQ_PROFILE_OVL_SWITCH_SLOT    (RSPQ_MAX_OVERLAY_COUNT + 4)
+#define RSPQ_PROFILE_IDLE_SLOT          RSPQ_PROFILE_OVERLAY_COUNT
+#define RSPQ_PROFILE_IDLE_RDP_SLOT      (RSPQ_PROFILE_OVERLAY_COUNT + 1)
+#define RSPQ_PROFILE_IDLE_SYNC_SLOT     (RSPQ_PROFILE_OVERLAY_COUNT + 2)
+#define RSPQ_PROFILE_RDPQ_SYNC_SLOT     (RSPQ_PROFILE_OVERLAY_COUNT + 3)
+#define RSPQ_PROFILE_OVL_SWITCH_SLOT    (RSPQ_PROFILE_OVERLAY_COUNT + 4)
 
 #endif
```
Shorter names to fit on screen:
```diff
diff --git a/src/rspq/rspq.c b/src/rspq/rspq.c
index 53817b9b..9cfba1e7 100644
--- a/src/rspq/rspq.c
+++ b/src/rspq/rspq.c
@@ -1440,12 +1440,12 @@ void rspq_profile_reset()
     rspq_int_write(RSPQ_CMD_PROFILE_FRAME);
     memset(&profile_data, 0, sizeof(profile_data));
 
-    profile_data.slots[RSPQ_PROFILE_IDLE_SLOT].name = "idle (CPU)";
-    profile_data.slots[RSPQ_PROFILE_IDLE_RDP_SLOT].name = "idle (RDP)";
-    profile_data.slots[RSPQ_PROFILE_IDLE_SYNC_SLOT].name = "idle (FULL_SYNC)";
-    profile_data.slots[RSPQ_PROFILE_RDPQ_SYNC_SLOT].name = "idle (RDPQCmd_SyncFull)";
-    profile_data.slots[RSPQ_PROFILE_OVL_SWITCH_SLOT].name = "overlay switching";
-    profile_data.slots[RSPQ_PROFILE_BUILTIN_SLOT].name = "builtin commands";
+    profile_data.slots[RSPQ_PROFILE_IDLE_SLOT].name       = "idle   CPU";
+    profile_data.slots[RSPQ_PROFILE_IDLE_RDP_SLOT].name   = "-      RDP";
+    profile_data.slots[RSPQ_PROFILE_IDLE_SYNC_SLOT].name  = "-FULL_SYNC";
+    profile_data.slots[RSPQ_PROFILE_RDPQ_SYNC_SLOT].name  = "-RDPQ CmFS";
+    profile_data.slots[RSPQ_PROFILE_OVL_SWITCH_SLOT].name = "ovl switch";
+    profile_data.slots[RSPQ_PROFILE_BUILTIN_SLOT].name    = "blt-in cmd";
 
     for (size_t i = 1; i < RSPQ_MAX_OVERLAY_COUNT; i++)
     {
```
