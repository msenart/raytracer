#include "Shader.h"
#include "OpenGLIncludes.h"

ShaderProgram::ShaderProgram() : _id(glCreateProgram()) {}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(_id);
}

std::string ShaderProgram::file2String(const std::string &filename)
{
  std::ifstream input(filename.c_str());
  if(!input)
    throw std::ios_base::failure("[Shader Program][file2String] Error: cannot open " + filename);
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string ShaderProgram::loadShaderWithIncludes(
    const std::string& filename,
    std::unordered_set<std::string>& alreadyIncluded
) {
  if (alreadyIncluded.count(filename))
    return "";

  alreadyIncluded.insert(filename);

  std::string source = file2String(filename);
  if (source.empty())
    return "";

  std::stringstream result;
  std::istringstream iss(source);
  std::string line;

  std::filesystem::path basePath = std::filesystem::path(filename).parent_path();

  while (std::getline(iss, line)) {
    if (line.find("#include") == 0) {
      size_t firstQuote = line.find('"');
      size_t lastQuote  = line.find('"', firstQuote + 1);

      if (firstQuote != std::string::npos && lastQuote != std::string::npos) {
        std::string includeFile =
            (basePath / line.substr(firstQuote + 1,
            lastQuote - firstQuote - 1)).string();

        result << loadShaderWithIncludes(includeFile, alreadyIncluded) << "\n";
      }
    } else {
      result << line << "\n";
    }
  }
  return result.str();
}

void ShaderProgram::loadShader(GLenum type, const std::string &shaderFilename)
{
  // Loads and compile a shader, before attaching it to a program
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::unordered_set<std::string> included;
  std::string shaderSourceString = loadShaderWithIncludes(shaderFilename,included); // Loads the shader source from a file to a C++ string
  if(shaderSourceString.empty()) {
    std::cerr << "No content in shader " << shaderFilename << std::endl;
    glDeleteShader(shader);
    return;
  }
  const GLchar *shaderSource = (const GLchar *) shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // Load the vertex shader source code
  glCompileShader(shader);      // THe GPU driver compile the shader
  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled) {
    GLsizei len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    GLchar *log = new GLchar[len+1];
    glGetShaderInfoLog(shader, len, &len, log);
    std::cerr << "Compilation error in shader " << shaderFilename << " : " << std::endl << log << std::endl;
    delete [] log;
    glDeleteShader(shader);
    return;
  }
  glAttachShader(_id, shader); // Set the vertex shader as the one ot be used with the program/pipeline
  glDeleteShader(shader);
}

void ShaderProgram::reload()
{
  GLuint oldProgram = _id;
  GLuint newProgram = glCreateProgram();

  _id = newProgram;

  if (!computeShaderPath.empty()) {
    loadShader(GL_COMPUTE_SHADER, computeShaderPath);
  } else {
    if (!vertexShaderPath.empty())
      loadShader(GL_VERTEX_SHADER, vertexShaderPath);

    if (!fragmentShaderPath.empty())
      loadShader(GL_FRAGMENT_SHADER, fragmentShaderPath);

    if (!geometryShaderPath.empty())
      loadShader(GL_GEOMETRY_SHADER, geometryShaderPath);

    if (!tessControlShaderPath.empty())
      loadShader(GL_TESS_CONTROL_SHADER, tessControlShaderPath);

    if (!tessEvalShaderPath.empty())
      loadShader(GL_TESS_EVALUATION_SHADER, tessEvalShaderPath);
  }
  link();
  use();
  glDeleteProgram(oldProgram);
}

std::shared_ptr<ShaderProgram> ShaderProgram::genShaderProgram(
    const std::string& vertexShader,
    const std::string& fragmentShader,
    const std::string& geometryShader,
    const std::string& tessControlShader,
    const std::string& tessEvalShader,
    const std::string& computeShader)
{
  auto shaderProgramPtr = std::make_shared<ShaderProgram>();

  if (!computeShader.empty())
  {
    shaderProgramPtr->loadShader(GL_COMPUTE_SHADER, computeShader);
    shaderProgramPtr->computeShaderPath = computeShader;
  }
  else
  {
    if (!vertexShader.empty()) {
      shaderProgramPtr->loadShader(GL_VERTEX_SHADER, vertexShader);
      shaderProgramPtr->vertexShaderPath = vertexShader;
    }

    if (!fragmentShader.empty()) {
      shaderProgramPtr->loadShader(GL_FRAGMENT_SHADER, fragmentShader);
      shaderProgramPtr->fragmentShaderPath = fragmentShader;
    }

    if (!geometryShader.empty()) {
      shaderProgramPtr->loadShader(GL_GEOMETRY_SHADER, geometryShader);
      shaderProgramPtr->geometryShaderPath = geometryShader;
    }

    if (!tessControlShader.empty()) {
      shaderProgramPtr->loadShader(GL_TESS_CONTROL_SHADER, tessControlShader);
      shaderProgramPtr->tessControlShaderPath = tessControlShader;
    }

    if (!tessEvalShader.empty()) {
      shaderProgramPtr->loadShader(GL_TESS_EVALUATION_SHADER, tessEvalShader);
      shaderProgramPtr->tessEvalShaderPath = tessEvalShader;
    }
  }

  shaderProgramPtr->link();
  shaderProgramPtr->use();

  return shaderProgramPtr;
}
