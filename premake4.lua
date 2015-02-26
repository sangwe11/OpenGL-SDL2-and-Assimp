solution "OpenGL-SDL2-and-Assimp"
	configurations { "Debug", "Release" }

	configuration { "Debug" }
		targetdir "bin/Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration { "Release" }
		targetdir "bin/Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

	if _ACTION == "clean" then
		os.rmdir(".\bin")
	end

	project "Demo"
		language "C++"
		kind "ConsoleApp"

		-- C++ on linux
		configuration { "linux" }
			buildoptions "-std=c++11"
		configuration {}

		files { "./**.h", "./**.cpp" }
		excludes { "./includes/**", "./librarys/**" }

		includedirs { "./includes/" }
		libdirs { "./librarys/" }

		-- Link to external libraries
		configuration { "windows" }
			links { "SDL/SDL2", "SDL/SDL2main", "SDL/SDL2_image", "opengl32", "GL/glew32", "ASSIMP/assimp" }
		configuration { "linux" }
			links { "SDL2", "SDL2main", "SDL2_image", "GL", "GLEW", "assimp" }
		configuration {}
		
		-- Post build commands
		configuration { "Debug", "windows" }
			postbuildcommands { "xcopy models bin\\Debug\\models /s /e /h /i /Y", "xcopy shaders bin\\Debug\\shaders /s /e /h /i /Y", "xcopy windows\\dlls bin\\Debug /s /e /h /i /Y" }
		configuration { "Release", "windows" }
			postbuildcommands { "xcopy models bin\\Release\\models /s /e /h /i /Y", "xcopy shaders bin\\Release\\shaders /s /e /h /i /Y", "xcopy windows\\dlls bin\\Release /s /e /h /i /Y" }
		configuration { "Debug", "linux" }
			postbuildcommands { "cp models bin/Debug/models -r", "cp shaders bin/Debug/shaders -r" }
		configuration { "Release", "linux" }
			postbuildcommands { "cp models bin/Release/models -r", "cp shaders bin/Release/shaders -r" }
		configuration {}