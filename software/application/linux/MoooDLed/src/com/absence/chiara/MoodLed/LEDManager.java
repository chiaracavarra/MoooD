package com.absence.chiara.MoodLed;

public class LEDManager {
	
	static {
//		System.loadLibrary("libusbled");
		System.load("/home/marco/eclipse-luna/workspace/UsbLedManager/Debug/libusbled.so");
	}

	public static native void switchOff();
	
	public static native void switchOn();
	
	// TODO: scale rgb values from 0-255 to 0-100
	public static native void setRGB();
	
	public static native void init();
	
	public static native void release();
		
}
