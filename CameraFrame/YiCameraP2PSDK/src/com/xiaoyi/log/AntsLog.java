package com.xiaoyi.log;

import android.util.Log;
import com.ants360.camera.sdk.BuildConfig;

public class AntsLog {
	private final static String TAG = "AntsLog";

    private static boolean DEBUG = BuildConfig.DEBUG;
	
	public final static int ANTS_LOG_NO_PRINT = -1;
    public final static int ANTS_LOG_INVALID_PARAMETER = -2;
	
	
    public static int v(String tag, String msg) {
        if(tag == null || msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	if(DEBUG) return Log.v(tag, msg);
    	else return ANTS_LOG_NO_PRINT; 	
    }    
	
	public static int d(String tag, String msg) {
        if(tag == null || msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	if(DEBUG) return Log.d(tag, msg);
    	else return ANTS_LOG_NO_PRINT; 		
    }
    
    public static int i(String tag, String msg) {
        if(tag == null || msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	if(DEBUG) return Log.i(tag, msg);
    	else return ANTS_LOG_NO_PRINT; 
    }
	
    public static int w(String tag, String msg) {
        if(tag == null || msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	if(DEBUG) return Log.w(tag, msg);
    	else return ANTS_LOG_NO_PRINT; 	
    }
    
    public static int e(String tag, String msg) {
        if(tag == null || msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	if(DEBUG) return Log.e(tag, msg);
    	else return ANTS_LOG_NO_PRINT; 	
    }

    public static int e(String tag, String msg, Throwable tr) {
        if(tag == null || msg == null || tr == null) return ANTS_LOG_INVALID_PARAMETER;
        if(DEBUG) return  Log.e(tag, msg, tr);
        else return ANTS_LOG_NO_PRINT;
    }
    
    
    // print it anyway.
    public static int V(String msg){
        if(msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	return Log.v(TAG, msg);
    }	
    
    public static int D(String msg){
        if(msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	return Log.d(TAG, msg);
    }
    
    public static int I(String msg){
        if(msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	return Log.i(TAG, msg);
    }
    
    public static int W(String msg){
        if(msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	return Log.w(TAG, msg);
    }
    
    public static int E(String msg){
        if(msg == null) return ANTS_LOG_INVALID_PARAMETER;
    	return Log.e(TAG, msg);
    }

    public static int E(String msg, Throwable tr){
        if(msg == null || tr == null) return ANTS_LOG_INVALID_PARAMETER;
        return Log.e(TAG, msg, tr);
    }

    public static void setDebug(boolean debug){
        DEBUG = debug;
    }
	
}
