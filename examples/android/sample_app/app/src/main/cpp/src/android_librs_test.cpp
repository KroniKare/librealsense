#include <jni.h>
#include <string>
#include <sstream>

#include <cassert>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <android/log.h>
#include <thread>


#include "../../../../../../../../include/librealsense2/rs.hpp"


bool isStreaming = false;

rs2::pipeline_profile profile;


bool running = false;
std::thread t;
rs2::pipeline p;
void *outputBufferDepth;
void *outputBufferColor;

void call_from_thread() {
    while (isStreaming) {

        auto frameset = p.wait_for_frames(600000);
        auto color = frameset.get_color_frame();
        if (color.get_data() != nullptr && outputBufferColor != NULL)
            memcpy((unsigned char *) outputBufferColor, color.get_data(),
                   static_cast<size_t>(color.get_height() * color.get_stride_in_bytes()));


        auto depth = frameset.get_depth_frame();
        if (depth.get_data() != nullptr && outputBufferDepth != NULL)
            memcpy((unsigned char *) outputBufferDepth, depth.get_data(),
                   static_cast<size_t>(depth.get_height() * depth.get_stride_in_bytes()));
//       auto infrared = frameset.get_infrared_frame(0);
//        if (infrared.get_data() != nullptr && outputBufferColor != NULL)
//            memcpy((unsigned char *) outputBufferColor, infrared.get_data(),
//                   infrared.get_height() * infrared.get_stride_in_bytes());
    }
}


extern "C"

JNIEXPORT jboolean JNICALL
Java_com_intel_realsense_android_MainActivity_librsStartStreaming(JNIEnv *env,
                                                                  jobject instance,
                                                                  jobject depthBuffer,
                                                                  jobject colorBuffer, jint w,
                                                                  jint h) {
    rs2::config cfg;
    outputBufferDepth = env->GetDirectBufferAddress(depthBuffer);
    outputBufferColor = env->GetDirectBufferAddress(colorBuffer);
    cfg.enable_stream(RS2_STREAM_DEPTH, w, h);
    //cfg.enable_stream(RS2_STREAM_INFRARED,0,0,RS2_FORMAT_BGRA8);
    cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_BGRA8); //TODO: Bug - DOESNT WORK!

    rs2::log_to_file(RS2_LOG_SEVERITY_DEBUG);
    profile = p.start(cfg);
    //auto sensor=profile.get_device().first<rs2::depth_stereo_sensor>();
    //sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_HIGH_ACCURACY); //TODO: Bug - DOESNT WORK!
    isStreaming = true;
    t = std::thread(call_from_thread);
    return static_cast<jboolean>(true);
}

extern "C"

JNIEXPORT jboolean JNICALL
Java_com_intel_realsense_android_MainActivity_librsStopStreaming(JNIEnv *env,
                                                                 jobject instance) {

    if (isStreaming) {
        isStreaming = false;
        t.join();
        p.stop();
    }
    return static_cast<jboolean>(true);
}
