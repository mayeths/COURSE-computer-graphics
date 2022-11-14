#include <stdio.h>
#include <array>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "log.h"
#include "Camera.hpp"
#include "Window.hpp"
#include "drawableObject.hpp"
// #include "texture.hpp"
// #include "buffers.hpp"
#include "model.hpp"

#include "GUI.hpp"

static void errorCallback(int error, const char* description)
{ 
    log_error("GLFW error %d: %s\n", error, description);
}

static void printInfomation()
{
    GLint GLmajor = 0, GLminor = 0, GLrev = 0, flags = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &GLmajor);
    glGetIntegerv(GL_MINOR_VERSION, &GLminor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &GLrev);
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    log_info("OpenGL version: %d.%d.%d", GLmajor, GLminor, GLrev);

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    log_info("GLFW version: %d.%d.%d", major, minor, rev);
}

std::array<GLuint, 8> texture;

bool LoadGLTextures(std::array<std::string, 6> skyboxTextures, std::string terrainTexture, std::string detailTexture)
{
    for (int i = 0; i < 6; i++) {
        texture[i] = SOIL_load_OGL_texture(
            skyboxTextures[i].c_str(), SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y
        );
    }
    texture[6] = SOIL_load_OGL_texture(
        terrainTexture.c_str(), SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y
    );
    texture[7] = SOIL_load_OGL_texture(
        detailTexture.c_str(), SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y
    );

    for (int i = 0; i < texture.size(); i++) {
        if (texture[i] == 0) {
            log_error("SOIL error: %s", SOIL_last_result());
            return false;
        }
    }

    for(int i=0;i<5;i++) {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    }

    glBindTexture(GL_TEXTURE_2D, texture[5]);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, texture[6]);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
 
    glBindTexture(GL_TEXTURE_2D, texture[7]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
 
    return true;
}

const float piover180 = 0.0174532925f;
float heading;
float xpos;
float zpos;
GLfloat yrot;
GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;
GLfloat lookupdown = 0.0f;
GLfloat roll=0;

void SkyBox(void)                                   //绘制天空盒及海面
{
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_CLIP_PLANE0);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glBegin(GL_QUADS);                              //前面
        glNormal3f( 0.0f, 0.0f,-1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-15.0f,  0.0f,  15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-15.0f, 30.0f,  15.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 15.0f, 30.0f,  15.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 15.0f,  0.0f,  15.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(GL_QUADS);                              //后面
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-15.0f,  0.0f, -15.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 15.0f,  0.0f, -15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 15.0f, 30.0f, -15.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-15.0f, 30.0f, -15.0f);
        
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);                             //顶面
        glNormal3f( 0.0f,-1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-15.0f, 30.0f, -15.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 15.0f, 30.0f, -15.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 15.0f, 30.0f,  15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-15.0f, 30.0f,  15.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);                             //右面
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 15.0f,  0.0f, -15.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 15.0f,  0.0f,  15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 15.0f, 30.0f,  15.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 15.0f, 30.0f, -15.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glBegin(GL_QUADS);                            //左面
        glNormal3f( 1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-15.0f,  0.0f, -15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-15.0f, 30.0f, -15.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-15.0f, 30.0f,  15.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-15.0f,  0.0f,  15.0f);        
    glEnd();
        
    glBindTexture(GL_TEXTURE_2D, texture[3]);   
    glBegin(GL_QUADS);                           //底面
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-15.0f, 0.0f, -15.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 15.0f, 0.0f, -15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 15.0f, 0.0f,  15.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-15.0f, 0.0f,  15.0f); 
    glEnd();
        
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, texture[5]);   //绘制半透明海面波浪
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //用于绘制半透明的海面波纹以与底面纹理形成天空倒影的效果
    glColor4f(1.0,1.0,1.0,0.6);
    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glTexCoord2f(15.0f, -roll+15.0f); glVertex3f(-15.0f, 0.0f, -15.0f);             //通过纹理坐标的改变实现波浪涌动效果
        glTexCoord2f( 0.0f, -roll+15.0f); glVertex3f( 15.0f, 0.0f, -15.0f);
        glTexCoord2f( 0.0f, -roll+ 0.0f); glVertex3f( 15.0f, 0.0f,  15.0f);
        glTexCoord2f(15.0f, -roll+ 0.0f); glVertex3f(-15.0f, 0.0f,  15.0f);
    glEnd();                                   //增加纹理滚动变量
    roll+=0.5f;
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Model::Render()                                 //绘制地形图并贴图
{
    glTranslatef(2.5f, 0.0f, -2.0f);
    glRotatef(-30, 0.0, 1.0, 0.0);
    glScalef(10.0f, 10.0f, 10.0f);

    glActiveTexture(GL_TEXTURE0);                    //激活texture[7],texture[6]两个纹理
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[6]);

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[7]);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);                 //将两个纹理组合以绘制地形的细节,设置纹理组合器的模式，用于对地形细节的纹理映射
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD_SIGNED);

    GLdouble equation[] = { 0.0,1.0,0.0,0.05 };
    glClipPlane(GL_CLIP_PLANE0, equation);                      //用于地形模型关于海水平面的裁剪；
    glEnable(GL_CLIP_PLANE0);

    for (int i = 0; i < faces.size(); ++i)
    {
        glBegin(GL_TRIANGLES);
        glNormal3f(faces[i].normal.x, faces[i].normal.y, faces[i].normal.z);
        for (int j = 0; j < 3; ++j)
        {
            glMultiTexCoord2f(GL_TEXTURE0, vertices[faces[i].vertices[j]].tx * 5, vertices[faces[i].vertices[j]].ty * 5);     //用于设置多重纹理的坐标值
            glMultiTexCoord2f(GL_TEXTURE1, vertices[faces[i].vertices[j]].tx, vertices[faces[i].vertices[j]].ty);//分别指定两个纹理的纹理坐标
            glVertex3d(vertices[faces[i].vertices[j]].x,
                vertices[faces[i].vertices[j]].y,
                vertices[faces[i].vertices[j]].z);
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}

void renderScene(void)             //实现整个画面的绘制
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-10.0f);
    glRotatef(5,1.0,0.0,0.0);
    GLfloat xtrans = -xpos;
    GLfloat ztrans = -zpos;
    GLfloat ytrans = -walkbias-0.25f;
    GLfloat sceneroty = 360.0f - yrot;
    glRotatef(lookupdown,1.0f,0,0);         //视线上下转动
    glRotatef(sceneroty,0,1.0f,0);          //视线左右转动
    glTranslatef(xtrans, ytrans, ztrans);   //摄像机坐标变化
    // gluLookAt (0.0,0.0,0.0,0.0,0.0,-1.0,0.0,1.0,0.0); 
    SkyBox();
}

int main(int argc, char* argv[]) {
    glfwSetErrorCallback([] (int error, const char* description) { 
        log_error("GLFW error %d: %s\n", error, description);
    });

    int success;
    Window window(success, 800, 600);
    if (!success) return -1;

    Model model;
    if (!model.LoadHeighMap("res/heightmap.bmp",10,1,10)) {
        return -1;
    }
    bool res = LoadGLTextures(
        {
            "res/SkyBox/SkyBox0.bmp",
            "res/SkyBox/SkyBox1.bmp",
            "res/SkyBox/SkyBox2.bmp",
            "res/SkyBox/SkyBox3.bmp",
            "res/SkyBox/SkyBox4.bmp",
            "res/SkyBox/SkyBox5.bmp",
        },
        "res/terrain-texture3.bmp",
        "res/detail.bmp"
    );
    if (!res) {
        return -1;
    }

    // glm::vec3 startPosition(0.0f, 800.0f, 0.0f);
    // Camera camera(startPosition);
    // window.camera = &camera;

    // sceneElements scene;
    // scene.cam = &camera;
    // drawableObject::scene = &scene;

    // GUI gui(window);
    // gui.subscribe(&water);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // gluPerspective(45.0f,1.0/1.0,0.0f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f,0.0f,0.0f,0.0f);

    glClearDepth(1.0f); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_POLYGON_SMOOTH_HINT); //几何反走样
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    printInfomation();
    const double fpsLimit = 1.0 / 60.0;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.continueLoop())
    {
        double now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// logic update
        window.processInput(deltaUpdateTime);

        ////// frame render
        if ((now - lastRenderTime) >= fpsLimit) {
            renderScene();
            // gui.update();
            // gui.draw();
            window.swapBuffersAndPollEvents();
            lastRenderTime = now;
        }

        lastUpdateTime = now;
    }

    return 0;
}
