package com.example.smy.vrplayer.VRView;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;

import com.example.smy.vrplayer.VRListener.GestureListener;
import com.example.smy.vrplayer.VRListener.VRPlayListener;
import com.example.smy.vrplayer.VRRender.BaseVRRender;
import com.example.smy.vrplayer.VRRender.VRPictureRender;
import com.example.smy.vrplayer.VRRender.VRVideoRender;
import com.example.smy.vrplayer.common.ScreenUtils;

import org.rajawali3d.vr.surface.VRSurfaceView;

/**
 * Created by SMY on 2016/6/22.
 */
public class CustomCardboardView extends VRSurfaceView {

    public interface OnCardboardViewClickListener{
        void onCardboardViewOnClicked();
    }

    BaseVRRender mRenderer;
    OnCardboardViewClickListener mCardboardViewClickListener;

    public CustomCardboardView(Context context)
    {
        super(context);
        init();
    }

    public CustomCardboardView(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        init();
    }

    private void init()
    {
        setSettingsButtonEnabled(false);
        setVRModeEnabled(false);
        GestureListener gestureListener= new GestureListener();
        gestureListener.setOnEventGesture(mOnEventGesture);
        gestureListener.setToucheMode(GestureListener.TOUCHEMODE.MODE_ZOOM_AND_MOVE);
        setOnTouchListener(gestureListener);
    }

    public void initRender(boolean videoRender){
        if(videoRender) {
            mRenderer = new VRVideoRender(getContext());
        } else {
            mRenderer = new VRPictureRender(getContext());
        }
        setRenderer(mRenderer);
    }

    public void setDataSource(String url)
    {
        if(mRenderer instanceof  VRVideoRender) {
            ((VRVideoRender)mRenderer).setVideoPath(url);
        }
    }

    public void setDataSource(Bitmap bitmap){
        if(mRenderer instanceof VRPictureRender){
            ((VRPictureRender)mRenderer).initBitmap(bitmap);
        }
    }

    public void setCardboardViewClickListener(OnCardboardViewClickListener listener)
    {
        mCardboardViewClickListener = listener;
    }

    @Override
    public void onDetachedFromWindow()
    {
        super.onDetachedFromWindow();
        if(mRenderer instanceof VRVideoRender) {
            ((VRVideoRender) mRenderer).onDestroy();
        } else if(mRenderer instanceof VRPictureRender){
            ((VRPictureRender) mRenderer).onDestroy();
        }
    }

    private int mWidth;
    private int mHeight;

    public void setScale(int width, int height){
        mWidth = width;
        mHeight = height;
    }

    public void drawPoint(){
        SurfaceHolder holder = getHolder();
        Canvas canvas = holder.lockCanvas();
        Paint p = new Paint();
        p.setAntiAlias(true);
        p.setColor(Color.WHITE);
        canvas.drawCircle(mWidth/2 - mWidth/6, mHeight-mHeight/4, 4.0f, p);
        canvas.drawCircle(mWidth   - mWidth/6, mHeight-mHeight/4, 4.0f, p);
        holder.unlockCanvasAndPost(canvas);
    }

    public void drawPointCircle(){
        SurfaceHolder holder = getHolder();
        Canvas canvas = holder.lockCanvas();
        canvas.drawColor(Color.TRANSPARENT);
        Paint p = new Paint();
        p.setAntiAlias(true);
        p.setColor(Color.WHITE);
        p.setStyle(Paint.Style.STROKE);
        //
        canvas.drawCircle(mWidth/2 - mWidth/6, mHeight-mHeight/4, 8.0f, p);
        canvas.drawCircle(mWidth   - mWidth/6, mHeight-mHeight/4, 8.0f, p);
        holder.unlockCanvasAndPost(canvas);
    }

    private GestureListener.OnEventGesture mOnEventGesture = new GestureListener.OnEventGesture() {
        @Override
        public void onZoomGesture(View v, float newDist, float oldDist) {
            if (!getVRMode())
            {
                mRenderer.onZoomGesture(newDist, oldDist);
            }
        }

        @Override
        public void onMoveGesture(View v, MotionEvent currentMoveEvent, float mTouchStartX, float mTouchStartY) {
            if(!getVRMode())
            {
                mRenderer.addGestureRotateAngle(
                        ScreenUtils.rawY2Angle((Activity)getContext(), currentMoveEvent.getRawY() - mTouchStartY),
                        ScreenUtils.rawX2Angle((Activity)getContext(), currentMoveEvent.getRawX() - mTouchStartX)
                );
            }
        }

        @Override
        public void onClick(View v, MotionEvent currentMoveEvent) {
            if(mCardboardViewClickListener != null)
            {
                mCardboardViewClickListener.onCardboardViewOnClicked();
            }
        }

        @Override
        public void onLeftRightGesture(View v, MotionEvent currentEventm, float startx, float starty, GestureListener.GestureState state) {

        }

        @Override
        public void onUpDownLeftGesture(View v, MotionEvent currentEventm, float startx, float starty, GestureListener.GestureState state) {

        }

        @Override
        public void onUpDownRightGesture(View v, MotionEvent currentEventm, float startx, float starty, GestureListener.GestureState state) {

        }
    };

    @Override
    public boolean dispatchTouchEvent(MotionEvent event)
    {
        getParent().requestDisallowInterceptTouchEvent(true);
        return super.dispatchTouchEvent(event);
    }

    public void resetCameraOrientation()
    {
        if(null != mRenderer) {
            mRenderer.resetCameraOrientation();
        }
    }

    public void changeMediaProgress(int progress)
    {
        if(mRenderer instanceof  VRVideoRender) {
            ((VRVideoRender)mRenderer).setMediaPlayerSeekTo(progress);
        }
    }

    public boolean isPlaying()
    {
        if(mRenderer instanceof  VRVideoRender) {
            return ((VRVideoRender)mRenderer).isPlaying();
        }
        return false;
    }

    public void pausePlay()
    {
        if(mRenderer instanceof  VRVideoRender) {
            mRenderer.onPause();
        }
    }

    public void startPlay()
    {
        if(mRenderer instanceof  VRVideoRender) {
            mRenderer.onResume();
        }
    }

    public void setRenderListener(VRPlayListener listener)
    {
        if(mRenderer instanceof  VRVideoRender) {
            ((VRVideoRender)mRenderer).setVideoPlayListener(listener);
        }
    }

    @Override
    public void onPause()
    {
        if(mRenderer != null) {
            mRenderer.onPause();
        }
        super.onPause();
    }

    @Override
    public void onResume()
    {
        if(mRenderer != null) {
            mRenderer.onResume();
        }
        super.onResume();
    }

    public void changeWatchType()
    {
        if(mRenderer != null) {
            mRenderer.changeWatchType();
        }
    }

    public boolean isUsingSensor()
    {
        if(mRenderer != null) {
            return mRenderer.mUseSensor;
        }
        return false;
    }

    public void setUseSensor(boolean enable){
        mRenderer.mUseSensor = enable;
    }
}
