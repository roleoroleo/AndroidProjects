package smy.com.vrplayer.objects.yuv;


import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import smy.com.vrplayer.common.GLHelpers;
import smy.com.vrplayer.common.ShaderProgram;
import smy.com.vrplayer.common.TransformHelper;
import smy.com.vrplayer.common.VrConstant;
import smy.com.vrplayer.objects.base.CombineParams;

/**
 * 用途：预览界面 全景模式渲染
 * 输入是YUV分量，并且是双鱼眼
 */
public class DualYUVRectAngle extends AbstractYUVObject {
    private TransformHelper mTransform = TransformHelper.getInstance();

    private int mSphereWidth;
    private int mSphereHeight;

    private ShaderProgram mShaderProgram;
    private final String mVertexShader =
            "precision mediump float;\n" +
                    "attribute vec4 vPosition;\n" +
                    "attribute vec2 aTextureCoordFront;\n" +/*材质坐标*/
                    "attribute vec2 aTextureCoordBack;\n" +/*材质坐标*/
                    "varying vec2 vTextureCoordFront;\n" +
                    "varying vec2 vTextureCoordBack;\n" +
                    "varying vec4 Position;\n" +
                    "void main() {\n" +
                    "gl_Position =  vPosition ;\n" +
                    "Position = vPosition;\n" +
                    "vTextureCoordFront = aTextureCoordFront;\n" +
                    "vTextureCoordBack = aTextureCoordBack;\n" +
                    "}\n";

    private final String mFragmentShader =
            "#define pi 3.14159265358979 \n" +
                    "precision mediump float;\n" +
                    "varying vec2 vTextureCoordFront;\n" +
                    "varying vec2 vTextureCoordBack;\n" +
                    "varying vec4 Position;\n" +
                    "uniform sampler2D tex_y;\n" +
                    "uniform sampler2D tex_u;\n" +
                    "uniform sampler2D tex_v;\n" +
                    "uniform float sphereWidth; \n" +
                    "void main() {\n" +
                    "vec4 RGBAFront;\n" +
                    "vec3 yuvF;\n" +
                    "vec3 rgbF;\n" +
                    "vec4 RGBABack;\n" +
                    "vec3 yuvB;\n" +
                    "vec3 rgbB;\n" +
                    "float xAxis = (1.0 + Position.x)*pi;\n" +
                    "float yAxis = (1.0 - Position.y)/2.0*pi;\n" +

                    "yuvF.x = texture2D(tex_y, vTextureCoordFront).r;\n" +
                    "yuvF.y = texture2D(tex_u, vTextureCoordFront).r - 0.5;\n" +
                    "yuvF.z = texture2D(tex_v, vTextureCoordFront).r - 0.5;\n" +
                    "rgbF = mat3( 1,       1,         1," +
                    "            0,       -0.39465,  2.03211," +
                    "            1.13983, -0.58060,  0) * yuvF;\n" +
                    "RGBAFront = vec4(rgbF, 1);\n" +

                    "yuvB.x = texture2D(tex_y, vTextureCoordBack).r;\n" +
                    "yuvB.y = texture2D(tex_u, vTextureCoordBack).r - 0.5;\n" +
                    "yuvB.z = texture2D(tex_v, vTextureCoordBack).r - 0.5;\n" +
                    "rgbB = mat3( 1,       1,         1," +
                    "            0,       -0.39465,  2.03211," +
                    "            1.13983, -0.58060,  0) * yuvB;\n" +
                    "RGBABack = vec4(rgbB, 1);\n" +

                    "float sphereY = sin(yAxis)*cos(xAxis);\n" +
                    "if(sphereY>-0.06 && sphereY<0.06){\n" +
                    "gl_FragColor = (0.5-(sphereY/0.12))*RGBAFront+ (sphereY/0.12+0.5)*RGBABack;\n" +
                    //"gl_FragColor = vec4((0.5+(yAxis/100.0))*RGBA.rgb,1);\n"+
                    "}\n" +
                    "   if(sphereY>=0.06){\n" +
                    "   gl_FragColor = RGBABack;\n" +
                    " }\n" +
                    "   if(sphereY<=-0.06){\n" +
                    "    gl_FragColor = RGBAFront;\n" +
                    "  }\n" +
                    "}\n";

    /*
     * @param nSlices how many slice in horizontal direction.
     *                The same slice for vertical direction is applied.
     *                nSlices should be > 1 and should be <= 180
     * @param x,y,z the origin of the sphere
     * @param r the radius of the sphere
     * @param viewRadius the radius of 2 picture
     * @param centerFront the center of the front camera
     * @param centerBack the center of the back camera
     *
     */
    public DualYUVRectAngle(CombineParams params) {/*关键是得到这些顶点坐标和纹理坐标*/
        super(params.out_width, params.out_height);
        mSphereWidth = params.out_width;
        mSphereHeight = params.out_height;

        int step = VrConstant.SPHERE_SAMPLE_STEP;
        int iMax = mSphereWidth / step + 1;
        int jMax = mSphereHeight / step + 1;
        int nVertices = iMax * jMax;
        
        FloatBuffer stFront = ByteBuffer.allocateDirect(nVertices * 2 * VrConstant.FLOAT_SIZE)
                .order(ByteOrder.nativeOrder()).asFloatBuffer();
        FloatBuffer stBack = ByteBuffer.allocateDirect(nVertices * 2 * VrConstant.FLOAT_SIZE)
                .order(ByteOrder.nativeOrder()).asFloatBuffer();

        mTransform.getSingleRectAngleVertices(mVertices, stFront, stBack,params);

        mVertices.position(0);
        stFront.position(0);
        stBack.position(0);

        mShaderProgram = new ShaderProgram(mVertexShader, mFragmentShader);
        /*顶点着色器属性及统一变量*/
        aPositionLocation = mShaderProgram.getAttribute("vPosition");

        aTextureCoordLocationFront = mShaderProgram.getAttribute("aTextureCoordFront");
        aTextureCoordLocationBack = mShaderProgram.getAttribute("aTextureCoordBack");

        mTextureUniformY = mShaderProgram.getUniform("tex_y");
        mTextureUniformU = mShaderProgram.getUniform("tex_u");
        mTextureUniformV = mShaderProgram.getUniform("tex_v");
        GLHelpers.initTexture(id_y, id_u, id_v);

        initDualSphereVAO(mVertices,stFront,stBack,mIndices);
    }

    @Override
    public void draw(float[] matrix, ByteBuffer yBuffer, ByteBuffer uBuffer, ByteBuffer vBuffer,
                     int frameWidth, int frameHeight) {
        bb_y = yBuffer;
        bb_u = uBuffer;
        bb_v = vBuffer;
        GLES20.glUseProgram(mShaderProgram.getShaderHandle());

        bb_y.position(0);
        bb_u.position(0);
        bb_v.position(0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLHelpers.checkGlError("glActiveTexture GL_TEXTURE0");
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, id_y[0]);
        GLHelpers.checkGlError("glBindTexture id_y");
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE,
                frameWidth, frameHeight, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, bb_y);
        GLHelpers.checkGlError("glTexImage2D bb_y");
        GLES20.glUniform1i(mTextureUniformY, 0);
        GLHelpers.checkGlError("glUniform1i mTextureUniformY");

        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLHelpers.checkGlError("glActiveTexture GL_TEXTURE1");
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, id_u[0]);
        GLHelpers.checkGlError("glBindTexture id_u");
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE,
                frameWidth / 2, frameHeight / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, bb_u);
        GLES20.glUniform1i(mTextureUniformU, 1);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
        GLHelpers.checkGlError("glActiveTexture GL_TEXTURE2");
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, id_v[0]);
        GLHelpers.checkGlError("glBindTexture id_v");
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE,
                frameWidth / 2, frameHeight / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, bb_v);
        GLES20.glUniform1i(mTextureUniformV, 2);

        drawDualSphereObject();
    }
}
