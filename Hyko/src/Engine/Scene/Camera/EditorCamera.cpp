#include "EditorCamera.h"

#include "Engine/Core/KeyKodes.h"
#include "Engine/Core/MouseButtons.h"
#include "Engine/Events/InputEvents.h"

#include "Engine/System/FileSystem/LogFiles.h"

// GL / Maths
#include <glm/gtc/matrix_transform.hpp>

// std
#include <iostream>

Hyko::OrthographicData Hyko::ECamera::m_orthoData;
Hyko::PerspectiveData  Hyko::ECamera::m_perspData;
Hyko::projType Hyko::ECamera::m_type;

glm::mat4 Hyko::ECamera::m_projectionMat;
glm::mat4 Hyko::ECamera::m_viewMat;
glm::vec2 Hyko::ECamera::m_position;

int Hyko::ECamera::m_zoomValue = 45;
int Hyko::ECamera::m_camSpeed	 = 2;

Hyko::ECamera::ECamera(Hyko::OrthographicData data, glm::vec2 position)
{
	setData(data);
	m_position = position;

	m_type = projType::Orthographic;

	Hyko::LogF::addMsgToLog("Editor ortho camera created");
}

Hyko::ECamera::ECamera(Hyko::PerspectiveData data, glm::vec2 position)
{
	setData(data);
	m_position = position;

	m_type = projType::Perspective;

	Hyko::LogF::addMsgToLog("Editor perspective camera created");
}

Hyko::ECamera::~ECamera()
{
	Hyko::LogF::addMsgToLog("Editor camera destroyed");
}

void Hyko::ECamera::setPosition(glm::vec2 newPos)
{
	m_position = newPos;
}

void Hyko::ECamera::setPosition(float x, float y)
{
	m_position = { x, y };
}

void Hyko::ECamera::setData(Hyko::OrthographicData data)
{
	m_orthoData.m_bottom = data.m_bottom;
	m_orthoData.m_left = data.m_left;
	m_orthoData.m_right = data.m_right;
	m_orthoData.m_top = data.m_top;

	m_type = projType::Orthographic;
}

void Hyko::ECamera::setData(Hyko::PerspectiveData data)
{
	m_perspData.m_aspect = data.m_aspect;
	m_perspData.m_fovY = data.m_fovY;
	m_perspData.m_zFar = data.m_zFar;
	m_perspData.m_zNear = data.m_zNear;

	m_type = projType::Perspective;
}

void Hyko::ECamera::setCameraSpeed(const int value)
{
	m_camSpeed = value;
}

void Hyko::ECamera::initProjection()
{
	switch (m_type) {
	case projType::Orthographic:
		m_projectionMat = glm::ortho(m_orthoData.m_left, m_orthoData.m_right, m_orthoData.m_bottom, m_orthoData.m_top, -1.0f, 1.0f);
		break;
	case projType::Perspective:
		m_projectionMat = glm::perspective(glm::radians(m_perspData.m_fovY), m_perspData.m_aspect, m_perspData.m_zNear, m_perspData.m_zFar);
		break;
	}

	m_viewMat = glm::lookAt(glm::vec3(m_position, 0.0f), glm::vec3(m_position, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Hyko::ECamera::updateInput(float dt)
{
	// Movement
	if (Hyko::Input::isMouseButtonPressed(Hyko::Mouse::HK_MOUSE_BUTTON_RIGHT)) {
		if (Hyko::Input::isKeyPressed(Hyko::Key::HK_KEYBOARD_W))
			setPosition(getPosition().x, getPosition().y + m_camSpeed * dt);
		if (Hyko::Input::isKeyPressed(Hyko::Key::HK_KEYBOARD_S))
			setPosition(getPosition().x, getPosition().y - m_camSpeed * dt);
		if (Hyko::Input::isKeyPressed(Hyko::Key::HK_KEYBOARD_A))
			setPosition(getPosition().x - m_camSpeed * dt, getPosition().y);
		if (Hyko::Input::isKeyPressed(Hyko::Key::HK_KEYBOARD_D))
			setPosition(getPosition().x + m_camSpeed * dt, getPosition().y);
		if (Hyko::Input::isKeyPressed(Hyko::Key::HK_KEYBOARD_LEFT_CONTROL))
			setPosition(0.0f, 0.0f);
	}

	// Zoom
	if (Hyko::Input::getMouseYOffset() >= 1 || Hyko::Input::getMouseYOffset() <= -1) {
		switch (m_type) {
		case projType::Perspective: // Perspective
			if (m_perspData.m_fovY >= 1.0f && m_perspData.m_fovY <= 120.0f) {
				m_zoomValue += -((Hyko::Input::getMouseYOffset() * 1000) * dt);
				m_perspData.m_fovY = m_zoomValue;
			}

			if (m_perspData.m_fovY > 120.0f)
				m_zoomValue = 120.0f;
			if (m_perspData.m_fovY < 1.0f)
				m_zoomValue = 1.0f;

			/*m_orthoData.m_bottom = -(m_perspData.m_fovY * 3.33f);
			m_orthoData.m_left = -(m_perspData.m_fovY * 3.33f);
			m_orthoData.m_top = m_perspData.m_fovY * 3.33f;
			m_orthoData.m_right = m_perspData.m_fovY * 3.33f;*/
			// 3.33 is a constant for 400 / 120
			// 400 is a max number for one side when ortho zoom
			// 120 is a max fov angle for perspective zoom

			break;

		case projType::Orthographic: //Orthographic
			m_orthoData.m_bottom += ((Hyko::Input::getMouseYOffset() * 1000) * dt);
			m_orthoData.m_left += ((Hyko::Input::getMouseYOffset() * 1000) * dt);
			m_orthoData.m_top -= ((Hyko::Input::getMouseYOffset() * 1000) * dt);
			m_orthoData.m_right -= ((Hyko::Input::getMouseYOffset() * 1000) * dt);

			if ((m_orthoData.m_bottom < -400.0f && m_orthoData.m_left < -400.0f) && (m_orthoData.m_right > 400.0f && m_orthoData.m_top > 400.0f)) {
				m_orthoData.m_bottom = -400.0f;
				m_orthoData.m_left = -400.0f;
				m_orthoData.m_right = 400.0f;
				m_orthoData.m_top = 400.0f;
			}
			if ((m_orthoData.m_bottom > -1.0f && m_orthoData.m_left > -1.0f) && (m_orthoData.m_right < 1.0f && m_orthoData.m_top < 1.0f)) {
				m_orthoData.m_bottom = -1.0f;
				m_orthoData.m_left = -1.0f;
				m_orthoData.m_right = 1.0f;
				m_orthoData.m_top = 1.0f;
			}

			m_perspData.m_fovY = m_orthoData.m_top / 3.33f;
			// 3.33 is a constant for 400 / 120
			// 400 is a max number for one side when ortho zoom
			// 120 is a max fov angle for perspective zoom

			break;
		}
	}
}

glm::mat4 Hyko::ECamera::updateView()
{
	m_viewMat = glm::translate(glm::mat4(1.0f), { m_position, m_type == projType::Orthographic ? 0.0f : 100.0f });
	m_viewMat = glm::inverse(m_viewMat);
	
	return m_viewMat;
}

glm::mat4 Hyko::ECamera::updateProjection()
{
	switch (m_type) {
	case projType::Orthographic:
		m_projectionMat = glm::ortho(m_orthoData.m_left, m_orthoData.m_right, m_orthoData.m_bottom, m_orthoData.m_top, -1.0f, 1.0f);
		break;
	case projType::Perspective:
		m_projectionMat = glm::perspective(glm::radians(m_perspData.m_fovY), m_perspData.m_aspect, m_perspData.m_zNear, m_perspData.m_zFar);
		break;
	}

	return m_projectionMat;
}

void Hyko::ECamera::swapProjection(Hyko::projType type)
{
	m_type = type;
	updateProjection();
	updateView();
}

void Hyko::ECamera::Resize(float width, float height)
{
	auto aspect = width / height;
	setData(PerspectiveData{ (float)m_zoomValue, aspect, 0.1f, 100.0f });
}
