#include <iostream>
#include <glad/glad.h>

#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <vector>
#include <algorithm>

void processInput(GLFWwindow window[0]);

static void ErrorCallback(int error, const char* description)
{
  std::cout << "Error: " << error << ", " << description << std::endl;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action
        , int mods)
{
  scancode;
  mods;
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

int main()
{
  std::vector<GLFWwindow*> window;
  GLuint vertBuffer, vertShader, fragShader, program;
  GLint pvmLocation, vposLocation, vcolLocation;


  if(!glfwInit())
  {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  int quantity = 2;

  for(int count = 0; count < 1; count++)
  {
    window.push_back(glfwCreateWindow(640, 480,
            std::string("glfw window" + count).c_str() ,
            nullptr,
            nullptr));
    glfwSetErrorCallback(ErrorCallback);

    glfwMakeContextCurrent(window[count]);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));


    if(!window[count])
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

  glfwSetKeyCallback(window[count], KeyCallback);

  glfwSwapInterval(1);

  }
  /////////////////////////////////////////////////////////////////////////////
  /*
   * To start drawing something, five OpenGL some input vertex data, OpenGL
   * only processes 3d coordinates when they're in specific range between -1
   * .0f, and 1.0f on all three axes. All coordinates within this range is
   * together is called normalized device coordinates. anything outside of
   * the range will not be displayed.
   *
   * Because we want to render a single triangle we want to specify a total
   * of three vertices with each vertex having a 3D position. We define them
   * in NDC in a float array.
   */
  /////////////////////////////////////////////////////////////////////////////
  float gap = 1.0f/8.0f;

  float vertices1[] = {
          // first triangle
          0.5f,  0.5f, 0.0f,  // top right
          0.5f, -0.5f, 0.0f,  // bottom right
          -0.5f,  0.5f, 0.0f,  // top left

  };

  float vertices2[] = {
          // second triangle
          0.5f - gap , -0.5f - gap , 0.0f - gap,  // bottom right
          -0.5f - gap, -0.5f - gap, 0.0f - gap,  // bottom left
          -0.5f - gap,  0.5f - gap, 0.0f - gap   // top left
  };

  /////////////////////////////////////////////////////////////////////////////
  /*
   * With Vertex data defined, we can store a large amount of it into GPU
   * memory by using vertex buffer objects. Once the data is in the graphics
   * card's memory the vertex shader has almost instant access tot he
   * vertices making it extremely fast.
   *
   * this buffer has a unique ID corresponding to that buffer, so we can
   * generate one with a buffer ID using the glGenBuffers function:
   */
  /////////////////////////////////////////////////////////////////////////////
  unsigned int VBO1, VBO2;
  glGenBuffers(1, &VBO1);
  glGenBuffers(2, &VBO2);

  /////////////////////////////////////////////////////////////////////////////
  /*
   *OpenGL has many types of buffer objects and the buffer type of a vertex
   * buffer object is GL_ARRAY_BUFFER. OpenGL allows us to bind to several
   * buffers at once as long as they have a different buffer type. We can
   * bind the newly created buffer to the GL_ARRAY_BUFFER target with the
   * glBindBuffer function:
   */
  /////////////////////////////////////////////////////////////////////////////
  glBindBuffer(GL_ARRAY_BUFFER, VBO1);

  /////////////////////////////////////////////////////////////////////////////
  /*
   * any buffer calls from this point will be used to configure the currently
   * bound buffer, which is VBO. Then we can make a call to the glBufferData
   * function that copies the previously defined vertex data into the
   * buffer's memory.
   */
  /////////////////////////////////////////////////////////////////////////////
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

  /////////////////////////////////////////////////////////////////////////////
  /*
   *glBufferData is a function specifically targeted to copy user-defined
   * data into the currently bound buffer. Its first argument is the type of
   * the buffer we want to copy data into: the vertex buffer object currently
   * bound to the simple sizeof the vertex data suffices. The third parameter
   * is the actual data we want to send. The fourth parameter specifies how
   * we want the graphics card to manage the given data. Which can take 3
   * forms.
   *  +GL_STREAM_DRAW - the data is set once and used my GPU most of the time
   *  +GL_STATIC_DRAW - the data is set only once and used many times.
   *  +GL_DYNAMIC_DRAW - the data is changed a lot and used many times
   *
   * The position data of the triangle does not change, is used a lot, and
   * stays the same for every render call so its usage type should best be
   * GL_STATIC_DRAW. If, for instance, one would have a buffer with data that
   * is likely to change frequently, a usage type of GL_DYNAMIC_DRAW ensures
   * the graphics card will place the data in memory that allows for faster
   * writes.
   *
   * for now we stored the vertex data within memory on the graphics card as
   * managed by VBO. Next we want to create a vertex and fragment shader that
   * actually processes this data.
   */
  /////////////////////////////////////////////////////////////////////////////



  /////////////////////////////////////////////////////////////////////////////
  /*
   * Vertex Shader
   *  OpenGL requires that we at least set up a vertex and fragment shader if
   *  we want to do rendering.
   *
   *  Here we configure two very simple shaders for drawing our first
   *  triangle. The first thing we need to do is write the vertex shader in
   *  the shader language GLSL and then compile this shader so we can use it
   *  in our application. this code is very basic
   *
   *  As you can see, GLSL looks similar to C. Each shader begins with a
   *  declaration of its version. Since OpenGL 3.3 and gigher the version
   *  numbers GLSL match the version of OpenGL. We also explicitly mention
   *  we're using core profile functionality.
   *
   *  Next we declare all the input vertex attributes in the vertex shader
   *  with the in keyword. Right now we only care about position data so we
   *  only need a single vertex attribute. GLSL has a vector datatype that
   *  contains 1 to 4 floats based on its postfix digit. Since each vertex
   *  has a 3D coordinate we create a vec3 input variable with the name aPos.
   *  We also specifically set the location of the input variable via layout
   *  (location = 0) and you'll later see that why we're going to need that
   *  location.
   *
   *  To set the output of the vertex shader we have to assign the position
   *  data to the predefined gl_Position variable which is a vec4 behind the
   *  scenes. Whatever we set gl_Position to will be used as the output of
   *  the vertex shader. Since our input is a vector of size 3 we have to
   *  cast this to a vector of size 4. We can do this by inspecting the vec3
   *  values inside the constructor of vec4 and set its w component to 1.0f
   *
   *  The current vertex shader is probably the most simple vertex shader we
   *  can imagine because we did no processing whatsoever on the input data
   *  and simply  forwarded it to the shader's output.
   *
   *  We take the source code for the vertex shader and store it in a const C
   *  string at the top the code file for now.
   */
  /////////////////////////////////////////////////////////////////////////////
  const char* vertexShaderSource = (
                                 "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;"
                                 "void main()"
                                 "{"
                                 "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
                                 "}");

  /////////////////////////////////////////////////////////////////////////////
  /*
   *In order for the OpenGL to use the shader it ahs to dynamically compile
   * it at run-time from its source code. The first thing we need to do is
   * create a shader object referenced by an ID. So we store the vertex
   * shader as an unsigned int and create the shader with glCreateShader:
   */
  /////////////////////////////////////////////////////////////////////////////
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);

  /////////////////////////////////////////////////////////////////////////////
  /*
   * We provide  the type of shader we want to create as an argument to
   * glCreateShader. Since we're creating a vertex shader we pass in
   * GL_VERTEX_SHADER.
   *
   * Next we attach the shader source code to the shader object and compile
   * the shader:
   *
   * The glShaderSource function takes the shader object to compile to as its
   * first argument. The second argument specifies how many strings we're
   * passing as source code, which is only one. The  third parameter is the
   * actual source code of the vertex shader and we can leave the 4th
   * parameter to nullptr.
   */

    /*
     * To check if the compilation was successful after the call to
     * glCompileShader, first define an integer to indicate success and a
     * storage container for the error messages(if any). Then we check if
     * compilation was successful with glGetShaderiv. If compilation failed,
     * we should retrieve the error message with glGetShaderInfoLog and print
     * the error message.
     *
     * If no errors were detected while compiling the vertex shader it is now
     * compiled.
     */
  /////////////////////////////////////////////////////////////////////////////
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

  if(!success)
  {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog <<
    std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Fragment Shader
   *  The fragment shader is the second and final shader we're going to
   *  create for rendering a triangle. The fragment shader is all about
   *  calculating the color output of your pixels. To keep things simple the
   *  fragment shader is all about calculating the color output of your
   *  pixels. To keep things simple the fragment shader will always output an
   *  orange-ish color.
   *
   *  Colors in computer graphics are represented as an array of 4 values:
   *  the red, green, blue and alpha (opacity) component, commonly
   *  abbreviated to RGBA.
   *
   *  The fragment shader only requires one output variable and that is a
   *  vector of size 4 that defines the final color output that we should
   *  calculate ourselves. We can declare output values with the out keyword,
   *  that we named FragColor. Next we simply assign a vec4 to the color
   *  output as an orange color with an alpha value of 1.0f.
   */
  /////////////////////////////////////////////////////////////////////////////

  const char* fragSource =
          ("#version 330 core\n"
           "out vec4 FragColor;"
           ""
           "void main()"
           "{"
           "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
           "}");

  const char* fragSource2 =
          ("#version 330 core\n"
           "out vec4 FragColor;"
           ""
           "void main()"
           "{"
           "  FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);"
           "}");
  /////////////////////////////////////////////////////////////////////////////
  /*
   * The process for compiling a fragment shader is similar to the vertex
   * shader, although this time we use the GL_FRAGMENT_SHADER constant as the
   * shader type:
   *
   * After this both shader will be compiled and the last thing to do will be
   * linking them together.
   */
  /////////////////////////////////////////////////////////////////////////////

  unsigned int fragmentShader;
  unsigned int fragmentShader2;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragSource, nullptr);
  glCompileShader(fragmentShader);

  glShaderSource(fragmentShader2, 1, &fragSource2, nullptr);
  glCompileShader(fragmentShader2);

  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER2::FRAGMENT2::COMPILATION_FAILED\n" << infoLog
    << std::endl;
  }
  /////////////////////////////////////////////////////////////////////////////
  /*
   * Shader program
   *  A shader program object is the final linked version of multiple shaders
   *  combined. To use the recently compiled shaders we have to link a shader
   *  program's shaders will be used when we issue render calls.
   *
   *  When linking the shaders into a program it links the outputs of each
   *  shader to the inputs of the next shader. This is also where you'll get
   *  linking errors if your outputs and inputs do not match.
   *
   *  Creating a program object is  easy
   *    The glCreateProgram function creates a program and returns the ID
   *    reference to the newly created program object.
   */
  /////////////////////////////////////////////////////////////////////////////
  unsigned int shaderProgram, shaderProgram2;
  shaderProgram = glCreateProgram();
  shaderProgram2 = glCreateProgram();


  /////////////////////////////////////////////////////////////////////////////
  /*
   *  Now we need to attach
   *    the previously compiled shaders to the program object and then link
   *    them with glLinkProgram:
   *
   *    This should be pretty self-explanatory, we attach the shaders tot eh
   *    program and link them via glLinkProgram
   */
  /////////////////////////////////////////////////////////////////////////////
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glAttachShader(shaderProgram2, vertexShader);
  glAttachShader(shaderProgram2, fragmentShader2);
  glLinkProgram(shaderProgram);
  glLinkProgram(shaderProgram2);


  /////////////////////////////////////////////////////////////////////////////
  /*
   * Just like shader compilation we can also check if linking a shader
   * program failed and retrieve the corresponding log. However, instead of
   * using glGetShaderiv and glGetShaderInfoLog we now use this:
   */
  /////////////////////////////////////////////////////////////////////////////
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(!success)
  {
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog <<
    std::endl;
  }
  glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
  if(!success)
  {
    glGetProgramInfoLog(shaderProgram2, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER2::PROGRAM2::LINKING_FAILED\n" << infoLog <<
              std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////
  /*
   * The result is a program object that we can activate by calling
   * glUseProgram with the newly created program object as its argument:
   *
   * Every shader and rendering call after glUseProgram will now use this
   * program object (and thus  the shaders). After not needing them anymore,
   * make sure to delete them with the function calls glDeleteShader (shaders);
   */
  /////////////////////////////////////////////////////////////////////////////
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glDeleteShader(fragmentShader2);
  glUseProgram(shaderProgram);
  glUseProgram(shaderProgram2);

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /*
   * Summary Initialization
   *  we have everything setup: we initialized the vertex data in a buffer
   *  using a vertex buffer object, set up a vertex and fragment shader and
   *  told OpenGL how to link the vertex data to the vertex shader's vertex
   *  attributes.
   *
   *  Drawing and object in OpenGL would now look something like this:
   *    // 0. copy our vertices array in a buffer for OpenGL to use
   *    glBindBuffer(GL_ARRAY_BUFFER, VBO);
   *    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
   *    GL_STATIC_DRAW);
   *    // 1. then set the vertex attributes pointers
   *    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
   *    (void*)0);
   *    glEnableVertexAttribArray(0);
   *    // 2. use our shader program when we want to render an object
   *    glUseProgram(shaderProgram);
   *    // 3. now draw the object
   *    someOpenGLFunctionThatDrawsOurTriangle();
   */
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /*
   * Drawing an object
   *  We have to repeat this process every time we want to draw an object.
   *  It may not look like that mush, but imagine if we have over 5 vertex
   *  attributes and perhaps hundreds of different objects(which is not uncommon)
   *  . Binding the appropriate buffer objects and configuring all vertex
   *  attributes for each of those objects quickly becomes a cumbersome
   *  process. we could store all these state configurations into an object
   *  and simply bind this object to restore its state.
   */
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /*
   * Vertex Array Object
   *  A vertex array object (also known as VAO) can be bound just like a
   *  vertex buffer object and any subsequent vertex attribute calls from
   *  that point on will be stored inside the VAO. This has the advantage
   *  that when configuring vertex attribute pointers you only have to make
   *  those calls once and whenever we want to draw the object, we can just
   *  bind the corresponding VAO. This makes switching between different
   *  vertex data and attribute configurations as easy as binding a different
   *  VAO. All the state we just set is stored inside the VAO.
   *    NOTE: Core OpenGL requires that we use a VAO so it know what to do
   *    with our vertex inputs. If we fail to bind a VAO, OpenGL will most
   *    likely refuse to draw anything.
   *
   *    a VAO stores the following
   *      + Calls to glEnableVertexAttribArray or glDisableVertexAttribArray.
   *      + Vertex attribute configurations via glVertexAttribPointer.
   *      + Vertex buffer objects associated with Vertex attributes by calls
   *      to glVertexAttribPointer.
   *    The process to generate a VAO looks similar to that of a VBO:
   */
  /////////////////////////////////////////////////////////////////////////////
  unsigned int VAO1, VAO2;
  glGenVertexArrays(1, &VAO1);
  glGenVertexArrays(2, &VAO2);

  /////////////////////////////////////////////////////////////////////////////
  /*
   * To use a VAO all you have to do is bind the VAO using glBindVertexArray.
   * From that point on we should bind/configure the corresponding VBO(s) and
   * attribute pointer(s) and then unbind the VAO for later use. As soon as
   * we want to draw an object, we simply bind the VAO with the prefered
   * settings before drawing the object and that is it.
   */
  /////////////////////////////////////////////////////////////////////////////
  glBindVertexArray(VAO1);

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Linking Vertex Attributes
   *  OpenGl does not yet know how it should interpret the vertex data in
   *  memory and how it should connect the vertex data to the vertex shader's
   *  attributes.
   *
   *  with the knowledge of vertex attributes we can tell OpenGL how it
   *  should interpret the vertex data(per vertex attribute) using
   *  glVertexAttribPointer:
   *
   *  The function glVertexAttribPointer has quite a few parameters so let's
   *  carefully walk through them:
   *
   *    The first parameter specifies which vertex attribute we want to
   *    configure.[Remember that we specified the location of the position
   *    vertex atrribut in the vertex shader with layout (location = 0). This
   *    sets the location of the vertex attribute to 0, and since we want to
   *    pass data to this vertex attribute, we pass in 0.
   *
   *    The next argument specifies the size of the vertex attribute.
   *
   *    The third argument specifies the type of the data which is GL_FLOAT
   *    (a vec* of floating point values).
   *
   *    The next argument specifies if we want the data to be normalized. If
   *    we're inputting integer data types(int, byte) and we've set this to
   *    GL_TRUE, the integer data is normalized to 0 (of -1 for signed data)
   *    and 1 when converted to float. This is not relevant for us so we'll
   *    leave this at GL_FALSE.
   *
   *    The fifth argument is known as the stride and tells us the space
   *    between consecutive vertex attributes. Since the next set of position
   *    data is located exactly 3 times the size of a float away we specify
   *    that value as the stride. Note that since we know that the array is
   *    tightly packed (there is no space between the next vertex attribute
   *    value) we could've also specified the stride as 0 to let OpenGL
   *    determine the stride( this only works when values are tightly packed).
   *    Whenever we have more vertex attributes we have to carefully define
   *    the spacing between each vertex attribute but we'll get to see more
   *    examples of that later on.
   *
   *    The last parameter is of type void* and thus requires that weird cast
   *    . This is the offset of where the position data begins in the buffer.
   *    Since the position data is at the start of the data array this value
   *    is just 0. We will explore this parameter in more detail later on.
   *
   *    Now that we specified how OpenGL should interpret the vertex data we
   *    should also enable the vertex attribute with
   *    glEnableVertexAttribArray giving the vertex attribute location as its
   *    argument; vertex attributes are disabled by default.
   */
  /////////////////////////////////////////////////////////////////////////////
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(VAO2);

  glBindBuffer(GL_ARRAY_BUFFER, VBO2);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  /////////////////////////////////////////////////////////////////////////////
  /*
   * Summary Drawing
   *  And that is it! a VAO that stores our vertex attribute configuration
   *  and which VBO to use. Usually when you have multiple objects you want
   *  to draw, you first generate/configure all the VAOs (and thus the
   *  required VBO and attribute pointers) and store those for later use. The
   *  moment we want to draw one of our objects, we take the corresponding
   *  VAO, bind it, then draw the object and unbind the VAO again.
   */
  /////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////
  /*
   * The triangle we've all been waiting for
   *  To draw our objects of choice, OpenGL provides us with the glDrawArrays
   *  function that draws primitives using the currently active shader, the
   *  previously defined vertex attribute configuration and with VBO's vertex
   *  data (indirectly bound via the VAO)
   *
   *  the glDrawArrays function takes as its first argument the OpenGL
   *  primitive type we would like to draw. We pass in GL_TRIANGLES. The second
   *  argument specifies the starting index of the vertex array we'd like to
   *  draw; we just leave this at 0. The last argument specifies how many
   *  vertices we want to draw, which is 3 (we only render 1 triangle from
   *  our data, which is exactly 3 vertices long).
   */
  /////////////////////////////////////////////////////////////////////////////



  double time = glfwGetTime();
  static bool allWindowsClosed = true;
  //render loop here
  do
  {
    allWindowsClosed = true;

    std::for_each(window.begin(), window.end(), [](GLFWwindow* theWindow)
    {
      if(theWindow == nullptr)
        return;

      if(glfwWindowShouldClose(theWindow))
      {
        glfwSetWindowShouldClose(theWindow, GLFW_TRUE);
      }
      else
      {

        allWindowsClosed = false;
      }
    });


    // input
    // -----


    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO1); // seeing as we only have a single VAO there's
    // no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(shaderProgram2);
    glBindVertexArray(VAO2);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // glBindVertexArray(0); // no need to unbind it every time

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window[0]);
    glfwPollEvents();


  }while(!allWindowsClosed);


  std::for_each(window.begin(), window.end(), glfwDestroyWindow);

  glfwTerminate();
  exit(EXIT_SUCCESS);


}