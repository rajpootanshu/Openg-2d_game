#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
using namespace std;
#include<bits/stdc++.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
//glm::mat4 VP = Matrices.projection * Matrices.view;

using namespace std;


//#define TRACE
//#ifdef TRACE
//#define tr(...) __f(#__VA_ARGS__, __VA_ARGS__)
//#else
//#define tr(...)
//#endif


struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

void draw();


/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);


    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed */
float rectangle_rotation = 0;
float triangle_rotation = 0;
float teer_rotation=0;
float increments = 1;
float camera_rotation_angle=90;
float x_cor_of_teer;
float y_cor_of_teer;
int flag=0; float t=0;
int flag1=0,flag2=0,v=8,flag8=0,flag11=0;
float zoom=1;
int width = 600;
int height = 600;
/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{

    switch (key) {
        case 'c':
            //  case 'C':
            // rectangle_rot_status = !rectangle_rot_status;
            //float increments = 1;
            //glutSwapBuffers ();

            // camera_rotation_angle++;

            triangle_rotation = triangle_rotation + 2;//increments*triangle_rot_dir*triangle_rot_status;
            rectangle_rotation = rectangle_rotation +2;
            teer_rotation = teer_rotation +2;
            // printf("%lf ",rectangle_rotation);
            break;

        case 'b':

            // inangle_rotation = triangle_rotation + 2;
            rectangle_rotation = rectangle_rotation - 2;
            triangle_rotation = triangle_rotation -2;
            teer_rotation = teer_rotation -2;

            break;
        case 'm':
            flag=1;
            break;
        case 'f':
            v=v+1;
            flag1=1;
	    break;

        case 's':
            v=v-1;
            flag2=1;
            break;
        case 'h':
            flag8=1;
            //flag11=1;
            break;
	case 'z':
		zoom+=0.1;
		break;
	case 'x':
		zoom-=0.1;
		break;

    }


}


/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{

}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
}

/* Executed when a mouse button 'button' is put into state 'state'
   at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_UP)
            {
                triangle_rotation  = triangle_rotation + 2;
                rectangle_rotation = rectangle_rotation + 2;
                teer_rotation= teer_rotation + 2;
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
               // rectangle_rotation  = rectangle_rotation -2;
                triangle_rotation =  triangle_rotation - 2;
                rectangle_rotation=rectangle_rotation-2;
                teer_rotation=teer_rotation -2;
            }
            break;
        default:
            break;
    }
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{
}
void reshapeWindow (int width, int height)
{
    GLfloat fov = 90.0f;
    glViewport (0, 0, (GLsizei) width, (GLsizei) height);
    Matrices.projection = glm::ortho(-4.0f*zoom, 4.0f*zoom, -4.0f*zoom, 4.0f*zoom, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *teer, *target1,* target2, *target3, *target4, *danger, *target5, *target6,*danger2,*target7;

// Creates the triangle object used in this sample code
void createTriangle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        0.6, 0.15,0, // vertex 0
        0,0.64,0, // vertex 1
        1,1.25,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 0
        0,1,0, // color 1
        0,0,1, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported static
    const GLfloat vertex_buffer_data [] = {
        0.6,0.15,0, // vertex 1
        0,-0.5,0, // vertex 2
        -0.5,0.03,0, // vertex 3

        0.6,0.15,0, // vertex 3
        0, 0.64,0, // vertex 4
        -0.5,0.03,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0, // color 3


        0,1,0, // color 3
        0.3,0.3,0.3, // color 4
        1,0,0  // color 1
    };

    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createTeer ()
{
    static const GLfloat vertex_buffer_data [] = {
        0.3,0.13,0,
        0,0.4,0,
        0.6,0.7,0,
    };
    static const GLfloat color_buffer_data [] = {
        0, 0 ,1,
        0 ,0, 1,
        0, 0, 1 ,
    };
    teer = create3DObject(GL_TRIANGLES , 3 , vertex_buffer_data , color_buffer_data, GL_FILL);


}
void createTarget1()
{
    static const GLfloat vertex_buffer_data [] = {
        3,2,0,
        3,2.4,0,
        2.4,2,0,

        3,2.4,0,
        2.4,2.4,0,
        2.4,2,0

    };
    static const GLfloat color_buffer_data [] = {
        1, 1 ,0,
        1 ,1, 0,
        1, 1, 0,

        1,1,0,
        1,1,0,
        1,1,0

    };
    target1 = create3DObject(GL_TRIANGLES , 6 , vertex_buffer_data , color_buffer_data, GL_FILL);
}

void createTarget2()
{
    static const GLfloat vertex_buffer_data [] = {
        3,0.6,0,
        3,1,0,
        2.4,1,0,

        3,0.6,0,
        2.4,0.6,0,
        2.4,1,0

    };
    static const GLfloat color_buffer_data [] = {
        1, 1 ,0, 
        1 ,1, 0,
        1, 1, 0,

        1,1,0,
        1,1,0,
        1,1,0

    };
    target2 = create3DObject(GL_TRIANGLES , 6 , vertex_buffer_data , color_buffer_data, GL_FILL);
}
void createTarget3()
{
    static const GLfloat vertex_buffer_data [] = {
        3,-0.4,0,
        3,-0.8,0,
        2.4,-0.8,0,

        3,-0.4,0,
        2.4,-0.4,0,
        2.4,-0.8,0

    };
    static const GLfloat color_buffer_data [] = {
        1, 1 ,0, 
        1 ,1, 0,
        1, 1, 0,

        1,1,0,
        1,1,0,
        1,1,0

    };
    target3 = create3DObject(GL_TRIANGLES , 6 , vertex_buffer_data , color_buffer_data, GL_FILL);
}
void createTarget4()
{
    static const GLfloat vertex_buffer_data [] = {
        3,-1.8,0,
        3,-2.2,0,
        2.4,-2.2,0,

        3,-1.8,0,
        2.4,-1.8,0,
        2.4,-2.2,0

    };
    static const GLfloat color_buffer_data [] = {
        1, 1 ,0, 
        1 ,1, 0,
        1, 1, 0,

        1,1,0,
        1,1,0,
        1,1,0

    };
    target4 = create3DObject(GL_TRIANGLES , 6 , vertex_buffer_data , color_buffer_data, GL_FILL);
}   
void createDanger()
{
    static const GLfloat vertex_buffer_data [] = {
        0,0.2,0,
        0,-0.3,0,
        -0.4,-0.3,0,

        0,0.2,0,
        -0.4,-0.3,0,
        -0.4,0.2,0

    };
    static const GLfloat color_buffer_data [] = {
        1, 0 ,0,
        1 ,0, 0,
        1, 0, 0,

        1,0,0,
        1,0,0,
        1,0,0

    };
    danger = create3DObject(GL_TRIANGLES , 6 , vertex_buffer_data , color_buffer_data, GL_FILL);
}
void createTarget5()
{
    static const GLfloat vertex_buffer_data []= {
        0.1,-0.5,0,
        0.1,-1.0,0,
        -0.3,-1.0,0,

        0.1,-0.5,0,
        -0.3,-0.5,0,
        -0.3,-1.0,0
    };
    static const GLfloat color_buffer_data []={
        1,1,0,
        1,1,0,
        1,1,0,

        1,1,0,
        1,1,0,
        1,1,0
    };
    target5 = create3DObject(GL_TRIANGLES ,6,vertex_buffer_data,color_buffer_data,GL_FILL);
}
void createTarget6()
{
    static const GLfloat vertex_buffer_data []= {
        0,-1.2,0,
        0,-1.6,0,
        -0.4,-1.6,0,

        0,-1.2,0,
        -0.4,-1.2,0,
        -0.4,-1.6,0
    };
    static const GLfloat color_buffer_data []={
        1,1,0,
        1,1,0,
        1,1,0,

        1,1,0,
        1,1,0,
        1,1,0
    };
    target6 = create3DObject(GL_TRIANGLES ,6,vertex_buffer_data,color_buffer_data,GL_FILL);
}
void createDanger2()
{
    static const GLfloat vertex_buffer_data []= {
        0.4,-1.9,0,
        0.4,-2.4,0,
        0,-2.4,0,

        0.4,-1.9,0,
        0,-1.9,0,
        0,-2.4,0,
    };
    static const GLfloat color_buffer_data []={
        1,0,0,
        1,0,0,
        1,0,0,

        1,0,0,
        1,0,0,
        1,0,0
    };
    danger2= create3DObject(GL_TRIANGLES ,6,vertex_buffer_data,color_buffer_data,GL_FILL);
}



int flag3=0,flag4=0,flag5=0,flag6=0,flag9=0,flag10=0;
int score=0,flag7=0;
float zoom_t=0;
void draw ()
{
    if(zoom!=zoom_t)
	{
		zoom_t=zoom;
		reshapeWindow (width, height);
		//fprintf(stderr,"%f\n",zoom);
	}
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram (programID);

    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    glm::vec3 target (0, 0, 0);
    glm::vec3 up (0, 1, 0);
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    glm::mat4 VP = Matrices.projection * Matrices.view;

    glm::mat4 MVP;	// MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateTriangle = glm::translate (glm::vec3(-3.5,-3.5, 0)); // glTranslatef
    glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


    draw3DObject(triangle);

    Matrices.model = glm::mat4(1.0f);

    glm::mat4 translateRectangle = glm::translate (glm::vec3(-3.5, -3.5, 0));        // glTranslatef
    glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    draw3DObject(rectangle);
    Matrices.model = glm::mat4(1.0f);
    // int flag3=0;

    // glm::mat4 translateRectangle = glm::translate (glm::vec3(-3.5, -3.5, 0));        // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    //Matrices.model *= (translateRectangle * rotateRectangle);
    if(flag3==0)
    {
        //Matrices.model = glm::mat4(1.0f);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(target1);
        //printf("aagya\n");
    }
    if(flag4==0)
    {
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(target2);
    }
    if(flag5==0)
    {
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(target3);
    }
    if(flag6==0)
    {
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(target4);
    }

    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1 ,GL_FALSE, &MVP[0][0]);
    draw3DObject(danger);
    if(flag8==1)
    {
        if(flag10==0)
        {
            MVP=VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID ,1,GL_FALSE, &MVP[0][0]);
            draw3DObject(target5);
        }
        if(flag9==0)
        {
            MVP =VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE ,&MVP[0][0]);
            draw3DObject(target6);
        }
       // if(flag11==1)
       // {

        MVP = VP *Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE, &MVP[0][0]);
        draw3DObject(danger2);
       // }
    }

    int reset=0;
    if(flag==0 || reset==1)
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateTeer = glm::translate (glm::vec3(-3.5,-3.5,0));
        glm::mat4 rotateTeer = glm::rotate((float)((teer_rotation*M_PI)/180.0f), glm::vec3(0,0,1));
        Matrices.model *= (translateTeer* rotateTeer );
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID ,1, GL_FALSE ,&MVP[0][0]);
        draw3DObject(teer);
    }
    if(flag==1&&(flag1==0||flag2==0))
    {
        // flag=0;
        Matrices.model =glm::mat4(1.0f);
        x_cor_of_teer = -3.5+ ( v*cos( (rectangle_rotation*M_PI)/180 + (45*M_PI)/180)   )*t ;
        y_cor_of_teer = -3.5+ ( v*sin( (rectangle_rotation*M_PI)/180 + (45*M_PI)/180)   )*t  - 5*t*t;
        //cout<<"y" <<y_cor_of_teer<<endl;
        //cout<< "v"<<v <<endl;

        if(y_cor_of_teer <= -10 )
        {
            x_cor_of_teer = -3.5; y_cor_of_teer = -3.5;
            t=0;flag=0;
            reset=1;
        }



        //tr(x_cor_of_teer,y_cor_of_teer);
        glm:: mat4 translateTeer = glm::translate (glm::vec3(x_cor_of_teer,y_cor_of_teer,0));
        Matrices.model *= (translateTeer );
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(teer);
        t = t+0.015;
    }



    if(flag==1&&(flag1==1||flag2==1))
    {
        // flag=0;
        Matrices.model =glm::mat4(1.0f);
        x_cor_of_teer = -3.5+ ( v*cos( (rectangle_rotation*M_PI)/180 + (45*M_PI)/180)   )*t ;
        y_cor_of_teer = -3.5+ ( v*sin( (rectangle_rotation*M_PI)/180 + (45*M_PI)/180)   )*t  - 5*t*t;
        //    cout<<"y" <<y_cor_of_teer<<endl;
        //  cout<< "v"<< v <<endl;

        if(y_cor_of_teer <= -10 )
        {
            x_cor_of_teer = -3.5; y_cor_of_teer = -3.5;
            t=0;flag=0;
            reset=1;
        }


        //tr(x_cor_of_teer,y_cor_of_teer);
        glm:: mat4 translateTeer = glm::translate (glm::vec3(x_cor_of_teer,y_cor_of_teer,0));
        Matrices.model *= (translateTeer );
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(teer);
        t = t+0.015;
    }
    if((2.4<=x_cor_of_teer)&&(x_cor_of_teer<=3)&&(2<=y_cor_of_teer)&&(y_cor_of_teer<=2.4))
    {
        flag3=1;
        score=score+10;
        cout << "your score" << score << endl;
        //printf("aahi nahi raha");
    }
    if((2.4<=x_cor_of_teer)&&(x_cor_of_teer<=3)&&(0.6<=y_cor_of_teer)&&(y_cor_of_teer<=1))

    {
        flag4=1;
        score=score+10;
        cout << "your score" << score << endl;


    }

    if((2.4<=x_cor_of_teer)&&(x_cor_of_teer<=3)&&(-0.8<=y_cor_of_teer)&&(y_cor_of_teer<=-0.4))
    {
        flag5=1;
        score=score+10;
        cout << "your score" << score << endl;

    }
    if((2.4<=x_cor_of_teer)&&(x_cor_of_teer<=3)&&(-2.2<=y_cor_of_teer)&&(y_cor_of_teer<=-1.8))
           {
                     flag6=1;
                    // score=score+10;
                    // cout << "your score" << score << endl;
             
                     }

    if((-0.4<=x_cor_of_teer)&&(x_cor_of_teer<=0)&&(-1.6<=y_cor_of_teer)&&(y_cor_of_teer<=-1.2))
    {
        flag9=1;
         score=score+10;
         cout << "your score" << score << endl;

    }


    if((-0.3<=x_cor_of_teer)&&(x_cor_of_teer<=0)&&(-1.0<=y_cor_of_teer)&&(y_cor_of_teer<=-0.5))
    {
        flag10=1;
         score=score+10;
        cout << "your score" << score << endl;

    }


    if((0<=x_cor_of_teer)&&(x_cor_of_teer<=0.4)&&(-2.4<=y_cor_of_teer)&&(y_cor_of_teer<=-1.9))
    {
        //cout << "you hit the DANGER" << endl;
        if(flag3==1)
        {
            flag3=0;
        }

        if(flag4==1)
        {
            flag4=0;
        }
        if(flag5==1)
        {
            flag5=0;
        }
        if(flag6==1)
        {
            flag6=0;
        }
        if(flag9==1)
        {
            flag9=0;
        }
        if(flag10==1)
        {
            flag10=0;
        }
      //  if(flag9==1)
       // {
         //   flag3=0;
       // }
        //if(flag3==1)
       // {
         //   flag3=0;
       // }
    }




    if((-0.4<=x_cor_of_teer)&&(x_cor_of_teer<=0)&&(-0.3<=y_cor_of_teer)&&(y_cor_of_teer<=0.2))
    {
        //cout << "you hit the Danger" << endl;
        if(flag3==1)
        {
            flag3=0;
            // score= score-10;
            //cout << "your score" << score << endl;
            //flag7=0;
        }


        if(flag4==1)
        {
            flag4=0;
            //score=score-10;
            //cout << "your score" << score << endl;

        }
        if(flag5==1)
        {
            flag5=0;
            // score=score-10;
            //cout << "your score" << score << endl;

        }
        if(flag6==1)
        {
            flag6=0;
            //score=score-10;
            //cout << "your score" << score << endl;


        }
        if(flag9==1)
        {
            flag9=0;
        }
        if(flag10=0)
        {
            flag10=0;
        }
    }
    //if(flag8==1)
    // {



    glutSwapBuffers ();
    //glm:: mat4 translateTeer = glm::translate ( glm::(-3.5,-3.5,0));
    //glm:: mat4 rotate 


}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)

    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
    // Create the models
    createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (width, height);

    // Background color of the scene
    glClearColor (1.5f, 2.3f, 3.5f, 1.5f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    createRectangle ();

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    createTeer ();
    createTarget1 ();
    createTarget2();
    createTarget3();
    createTarget4();
    createDanger();
    createTarget5();
    createTarget6();
    createDanger2();
    if(flag3==1&&flag4==1&&flag5==1&&flag6==1)
    {
        cout << "you win" << endl;
    }
    if(flag4==1)
    {
        score=score+10;
        cout<<"score" << score << endl;
        flag4=0;
    }
    if(flag5==1)
    {
        score=score+10;
        cout<<"score" << score <<endl;
        flag5=0;
    }
    if(flag6==1)
    {
        score=score+10;
        cout <<"score"<<  score << endl;
        flag6=0;
    }
    if(flag3==1)
    {
        score=score+10;
        cout<< "score "<< score <<endl;
        flag3=0;
    }

}

int main (int argc, char** argv)
{
    int height=650;
    int width=650;
    

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

    initGL (width, height);

    glutMainLoop ();

    return 0;
}
