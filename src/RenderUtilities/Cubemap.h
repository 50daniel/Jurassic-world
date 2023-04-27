#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>


class Cubemap
{
public:
	enum Type {
		TEXTURE_DEFAULT = 0,
		TEXTURE_DIFFUSE, TEXTURE_SPECULAR,
		TEXTURE_NORMAL, TEXTURE_DISPLACEMENT,
		TEXTURE_HEIGHT,
	};

	Type type;

	Cubemap(GLuint* cubemapID)
		//type(texture_type)
	{

		cv::Mat img[6];
		//cv::imread(path, cv::IMREAD_COLOR).convertTo(img, CV_32FC3, 1 / 255.0f);	//unsigned char to float

			img[0] = cv::imread(PROJECT_DIR "/Images/skybox/right.jpg", cv::IMREAD_COLOR);
			img[1] = cv::imread(PROJECT_DIR "/Images/skybox/left.jpg", cv::IMREAD_COLOR);
			img[2] = cv::imread(PROJECT_DIR "/Images/skybox/top.jpg", cv::IMREAD_COLOR);
			img[3] = cv::imread(PROJECT_DIR "/Images/skybox/bottom.jpg", cv::IMREAD_COLOR);
			img[4] = cv::imread(PROJECT_DIR "/Images/skybox/front.jpg", cv::IMREAD_COLOR);
			img[5] = cv::imread(PROJECT_DIR "/Images/skybox/back.jpg", cv::IMREAD_COLOR);
		
		
			for (int i = 0; i < 6; i++)
			{
				this->size[i].x = img[i].cols;
				this->size[i].y = img[i].rows;
			}
		
		
		//cv::cvtColor(img, img, CV_BGR2RGB);
		GLuint textureID;
		glGenTextures(1, &this->id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
		int width, height;
		unsigned char* image;
		for (GLuint i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, img[i].cols, img[i].rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img[i].data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		/*glGenTextures(1, &this->id);

		glBindTexture(GL_TEXTURE_2D, this->id);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (img.type() == CV_8UC3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
		else if (img.type() == CV_8UC4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.cols, img.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);
		glBindTexture(GL_TEXTURE_2D, 0);*/

		for (int i = 0; i < 6; i++)
		{
			img[i].release();
		}
		
	}
	void bind(GLenum bind_unit)
	{

		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
	}
	static void unbind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	
	glm::ivec2 size[6];
private:
	GLuint id;

};