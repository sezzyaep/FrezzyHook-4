#pragma once
#include "DLL_MAIN.h"

//struct matrix
//{
//	matrix()
//	{
//	}
//
//	matrix(
//		float m00, float m01, float m02, float m03,
//		float m10, float m11, float m12, float m13,
//		float m20, float m21, float m22, float m23)
//	{
//		m_flMatVal[0][0] = m00;
//		m_flMatVal[0][1] = m01;
//		m_flMatVal[0][2] = m02;
//		m_flMatVal[0][3] = m03;
//		m_flMatVal[1][0] = m10;
//		m_flMatVal[1][1] = m11;
//		m_flMatVal[1][2] = m12;
//		m_flMatVal[1][3] = m13;
//		m_flMatVal[2][0] = m20;
//		m_flMatVal[2][1] = m21;
//		m_flMatVal[2][2] = m22;
//		m_flMatVal[2][3] = m23;
//	}
//
//	void Init(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin)
//	{
//		m_flMatVal[0][0] = xAxis.x;
//		m_flMatVal[0][1] = yAxis.x;
//		m_flMatVal[0][2] = zAxis.x;
//		m_flMatVal[0][3] = vecOrigin.x;
//		m_flMatVal[1][0] = xAxis.y;
//		m_flMatVal[1][1] = yAxis.y;
//		m_flMatVal[1][2] = zAxis.y;
//		m_flMatVal[1][3] = vecOrigin.y;
//		m_flMatVal[2][0] = xAxis.z;
//		m_flMatVal[2][1] = yAxis.z;
//		m_flMatVal[2][2] = zAxis.z;
//		m_flMatVal[2][3] = vecOrigin.z;
//	}
//
//	matrix(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin)
//	{
//		Init(xAxis, yAxis, zAxis, vecOrigin);
//	}
//
//	float* operator[](int i)
//	{
//		return m_flMatVal[i];
//	}
//
//	const float* operator[](int i) const
//	{
//		return m_flMatVal[i];
//	}
//
//	float* Base()
//	{
//		return &m_flMatVal[0][0];
//	}
//
//	const float* Base() const
//	{
//		return &m_flMatVal[0][0];
//	}
//
//	float m_flMatVal[3][4];
//};

//typedef float matrix[3][4];
//typedef float matrix4x4[4][4];