#ifndef FOLLOWCAMERA_H
#define FOLLOWCAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace fcamera {
	const float YAW = 0.0f;
	const float PITCH = 0.0f;
	const float SPEED = 5.0f;
	const float SENSITIVITY = 0.1f;
	const float ZOOM = 45.0f;

	class FollowCamera {
	public:
		glm::vec3 Position;
		glm::vec3 Target;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		float Yaw;
		float Pitch;
		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;
		float Distance;

		FollowCamera(glm::vec3 position = glm::vec3(0.0f, 5.0f, 2.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
			// �n��w����H��m
			Target = target;

			// ��v������m
			Position = position;

			// �p����v���P�ؼФ������Z��
			Distance = glm::length(Position - Target);

			// �]�w��v���Ѽ�
			WorldUp = up;
			Yaw = yaw;
			Pitch = pitch;

			// ��s�Ѽ�
			updateCameraPosition();
		}

		glm::mat4 GetViewMatrix() {
			return calcLookAtMatrix(Position, Target, WorldUp);
		}

		// ��ڭ̭n��w���ؼЦ�m���ʮɡA��v���]�n��ۤ@�_����
		void updateTargetPosition(glm::vec3 target) {
			Target = target;
			updateCameraPosition();
		}

		// �t�d�B�z�ƹ��ƥ�]��v������ۥؼЪ�����V�^
		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			Yaw += xoffset;
			if (Yaw > 360) {
				Yaw -= 360;
			}
			if (Yaw < -360) {
				Yaw += 360;
			}
			Pitch += yoffset;
			if (Pitch > 360) {
				Pitch -= 360;
			}
			if (Pitch < -360) {
				Pitch += 360;
			}

			if (constrainPitch) {
				if (Pitch > 89.0f) {
					Pitch = 89.0f;
				}
				if (Pitch < -89.0f) {
					Pitch = -89.0f;
				}
			}

			// �O�o�A�]����v���P���骺�Z���O�T�w���A�ҥH��A�������o�{�ܤƮɡA�A����m�]�|��ۤ��P�C
			updateCameraPosition();
		}

		// �Y�u�ΩԪ���v���P���餧�����Z��
		void AdjustDistance(float yoffset) {
			if (Distance >= 3.0f && Distance <= 20.0f) {
				Distance -= (float)yoffset;
			}
			if (Distance < 3.0f) {
				Distance = 3.0f;
			}
			if (Distance > 20.0f) {
				Distance = 20.0f;
			}
			updateCameraPosition();
		}

		// �B�z�ƹ��u���ƥ�A�i�H�Y�u�μW�[FOV�C
		void ProcessMouseScroll(float yoffset) {
			if (Zoom >= 1.0f && Zoom <= 90.0f) {
				Zoom -= (float)yoffset;
			}
			if (Zoom < 1.0f) {
				Zoom = 1.0f;
			}
			if (Zoom > 90.0f) {
				Zoom = 90.0f;
			}
		}

	private:
		void updateCameraPosition() {
			glm::vec4 radius = glm::vec4(0.0f, 0.0f, Distance, 1.0f);

			glm::mat4 rotateMatrix = glm::mat4(1.0f);
			rotateMatrix = glm::rotate(rotateMatrix, glm::radians(-Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			rotateMatrix = glm::rotate(rotateMatrix, glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));

			glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(Target.x, Target.y, Target.z));

			glm::vec4 pos = translation * rotateMatrix * radius;

			Position = glm::vec3(pos.x, pos.y, pos.z);
			
			// ��X��v������m����A�N���X��v�����y�жb
			updateCameraVectors();
		}

		void updateCameraVectors() {
			Front = glm::normalize(Position - Target);
			Right = glm::normalize(glm::cross(WorldUp, Front));
			Up = glm::normalize(glm::cross(Front, Right));
		}

		glm::mat4 calcLookAtMatrix(glm::vec3 positon, glm::vec3 target, glm::vec3 worldUp) {
			glm::vec3 zaxis = glm::normalize(positon - target);
			glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(WorldUp), zaxis));
			glm::vec3 yaxis = glm::normalize(glm::cross(zaxis, xaxis));

			glm::mat4 translation = glm::mat4(1.0f);
			translation[3][0] = -positon.x;
			translation[3][1] = -positon.y;
			translation[3][2] = -positon.z;

			glm::mat4 rotation = glm::mat4(1.0f);
			rotation[0][0] = xaxis.x;
			rotation[1][0] = xaxis.y;
			rotation[2][0] = xaxis.z;
			rotation[0][1] = yaxis.x;
			rotation[1][1] = yaxis.y;
			rotation[2][1] = yaxis.z;
			rotation[0][2] = zaxis.x;
			rotation[1][2] = zaxis.y;
			rotation[2][2] = zaxis.z;

			return rotation * translation;
		}
	};
}
#endif // !FOLLOWCAMERA_H