#include "shaders.h"

	bool MyShader::OpenMyShader(const char* vertexPath, const char* fragmentPath) {

		std::string codeVertex = ReadMyShaderCode(vertexPath);
		if (codeVertex == "") {
			return false;
		}
		std::string codeFragment = ReadMyShaderCode(fragmentPath);
		if (codeFragment == "") {
			return false;
		}
		std::cout << "Compiling " << vertexPath << std::endl;
		unsigned int vMyShader = CompileMyShader(codeVertex.c_str(), GL_VERTEX_SHADER, "VERTEX");
		if (vMyShader == 0) {
			return false;
		}

		std::cout << "Compiling " << fragmentPath << std::endl;
		unsigned int fMyShader = CompileMyShader(codeFragment.c_str(), GL_FRAGMENT_SHADER, "FRAGMENT");
		if (fMyShader == 0) {
			return false;
		}

		// crée le programme de MyShader
		ID = glCreateProgram();
		glAttachShader(ID, vMyShader);
		glAttachShader(ID, fMyShader);
		glLinkProgram(ID);
		bool s = CheckCompileErrors(ID, "PROGRAM");
		glDeleteShader(vMyShader);
		glDeleteShader(fMyShader);
		return s;
	}

	Shrapper MyShader::GetShaderID() {
		return Shrapper(ID);
	}

	void MyShader::PrintErrorStack() {
		std::cout << ErrorMessage << std::endl;
	}


	// ReadMyShaderCode lit le code du MyShader depuis son fichier et le retourne
	std::string MyShader::ReadMyShaderCode(const char * filePath) {
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			// ouvre les fichier
			file.open(filePath);

			// lit l'intérieur dans un stream
			std::stringstream stream;
			stream << file.rdbuf();

			// close les file handle
			file.close();
			return stream.str();
		}
		catch (std::ifstream::failure e) {
			strcpy(ErrorMessage, "ERROR MyShader file not succesfully read");
		}
		return "";
	}
	// CompileMyShader compile le MyShader
	unsigned int MyShader::CompileMyShader(std::string code, GLenum type, std::string nameType) {
		unsigned int item;
		int success;

		const char* sPtr = code.c_str();
		const int sSize = code.size();

		item = glCreateShader(type);
		glShaderSource(item, 1, &sPtr, &sSize);
		glCompileShader(item);
		if (CheckCompileErrors(item, nameType)) {
			return item;
		}
		// retourne 0 si la compilation a fail
		return 0;
	}

	bool MyShader::CheckCompileErrors(GLuint MyShader, std::string nameType) {
		GLint success;
		char infoLog[1024];
		if (nameType != "PROGRAM") {
			glGetShaderiv(MyShader, GL_COMPILE_STATUS, &success);
			if (success == GL_FALSE) {
				int infoLength = 0;
				glGetShaderiv(MyShader, GL_INFO_LOG_LENGTH, &infoLength);
				glGetShaderInfoLog(MyShader, infoLength, NULL, infoLog);
				std::cerr << "ERROR Compilation of type : " << nameType << std::endl << infoLog << std::endl;
				return false;
			}
		}
		else {
			glGetProgramiv(MyShader, GL_LINK_STATUS, &success);
			if (success == GL_FALSE) {
				GLint maxLen = 0;
				glGetProgramInfoLog(MyShader, 1024, &maxLen, infoLog);
				std::string s(infoLog);
				std::cerr << "ERROR Compilation of typw : " << nameType << std::endl << infoLog << std::endl;
				strcpy(ErrorMessage, infoLog);
				return false;
			}
		}
		return true;
	}