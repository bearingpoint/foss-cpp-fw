#ifndef __DefaultShaderPreprocessor__h_
#define __DefaultShaderPreprocessor__h_

#include <boglfw/renderOpenGL/shader.h>

// this class handles the #include directives from shaders and resolves them by copy-pasting the contents
class DefaultShaderPreprocessor : public IShaderPreprocessor {
public:
	std::string preprocess(std::string const& code, std::string const& originalFilePath) override;

protected:
	// compute a full path from [relativePath] and the full path of another file that refers to the relative path
	std::string computePath(std::string const& relativePath, std::string const& fullReferrerPath);
};

#endif // __DefaultShaderPreprocessor__h_
