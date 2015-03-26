#include <jni.h>
#include <jni_md.h>

#include "ledmanager.h"

extern void switch_off();
extern void switch_on();
extern void set_rgb();
extern void release();
extern void init();


/*
 * Class:     com_absence_chiara_MoodLed_LEDManager
 * Method:    switchOff
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_absence_chiara_MoodLed_LEDManager_switchOff (JNIEnv *env, jclass class)
{
	switch_off();
}

/*
 * Class:     com_absence_chiara_MoodLed_LEDManager
 * Method:    switchOn
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_absence_chiara_MoodLed_LEDManager_switchOn (JNIEnv *env, jclass class)
{
	switch_on();
}

/*
 * Class:     com_absence_chiara_MoodLed_LEDManager
 * Method:    setRGB
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_absence_chiara_MoodLed_LEDManager_setRGB (JNIEnv *env, jclass class)
{
	set_rgb();
}

/*
 * Class:     com_absence_chiara_MoodLed_LEDManager
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_absence_chiara_MoodLed_LEDManager_init (JNIEnv *env, jclass class)
{
	init();
}

/*
 * Class:     com_absence_chiara_MoodLed_LEDManager
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_absence_chiara_MoodLed_LEDManager_release (JNIEnv *env, jclass class)
{
	release();
}
