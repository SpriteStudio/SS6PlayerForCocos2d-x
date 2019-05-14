LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := MyGame_shared

LOCAL_MODULE_FILENAME := libMyGame

LOCAL_SRC_FILES := $(LOCAL_PATH)/hellocpp/main.cpp \
                   $(LOCAL_PATH)/../../../Classes/AppDelegate.cpp \
                   $(LOCAL_PATH)/../../../Classes/HelloWorldScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/SS6Player.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/SS6PlayerPlatform.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/SS6PlayerPlatform.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/SS6PlayerPlatform.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/Common/Animator/ssplayer_effectfunction.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/Common/Animator/ssplayer_matrix.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/Common/Animator/ssplayer_effect2.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/Common/Animator/ssplayer_effect.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/Common/Animator/ssplayer_PartState.cpp \
                   $(LOCAL_PATH)/../../../Classes/SSPlayer/Common/Helper/DebugPrint.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../Classes \
                    $(LOCAL_PATH)/../../../Classes/SSPlayer \
                    $(LOCAL_PATH)/../../../Classes/SSPlayer/Common \
                    $(LOCAL_PATH)/../../../Classes/SSPlayer/Animator \
                    $(LOCAL_PATH)/../../../Classes/SSPlayer/Helper \
                    $(LOCAL_PATH)/../../../Classes/SSPlayer/Loader

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END


LOCAL_STATIC_LIBRARIES := cc_static

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module, cocos)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
