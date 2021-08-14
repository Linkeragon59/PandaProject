#include "RenderModel.h"

namespace Render
{
#if DEBUG_BUILD
	void DynamicModelData::GetVectorBaseWidget(std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
	{
		const float arrowLength = 1.0f;
		const float arrowDiameter = 0.01f;
		const uint arrowPrecision = 12;
		const float arrowAngle = 2.0f * 3.1416f / arrowPrecision;

		// 3 arrows
		someOutVertices.resize(3 * arrowPrecision * 2);
		someOutIndices.resize(3 * arrowPrecision * 2 * 3);

		for (uint i = 0; i < arrowPrecision; ++i)
		{
			const float sin = arrowDiameter * std::sin(i * arrowAngle);
			const float cos = arrowDiameter * std::cos(i * arrowAngle);

			// x
			someOutVertices[i].myPosition = glm::vec3(0.0f, sin, cos);
			someOutVertices[i].myColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			someOutVertices[i + arrowPrecision].myPosition = glm::vec3(arrowLength, sin, cos);
			someOutVertices[i + arrowPrecision].myColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

			// y
			const uint yOffset = arrowPrecision * 2;
			someOutVertices[i + yOffset].myPosition = glm::vec3(cos, 0.0f, sin);
			someOutVertices[i + yOffset].myColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			someOutVertices[i + yOffset + arrowPrecision].myPosition = glm::vec3(cos, arrowLength, sin);
			someOutVertices[i + yOffset + arrowPrecision].myColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

			// z
			const uint zOffset = 2 * arrowPrecision * 2;
			someOutVertices[i + zOffset].myPosition = glm::vec3(sin, cos, 0.0f);
			someOutVertices[i + zOffset].myColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			someOutVertices[i + zOffset + arrowPrecision].myPosition = glm::vec3(sin, cos, arrowLength);
			someOutVertices[i + zOffset + arrowPrecision].myColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}

		for (uint i = 0; i < arrowPrecision; ++i)
		{
			// x
			someOutIndices[3 * i]							= i;
			someOutIndices[3 * i + 1]						= (i + 1) % arrowPrecision;
			someOutIndices[3 * i + 2]						= i + arrowPrecision;
			someOutIndices[3 * (i + arrowPrecision)]		= i + arrowPrecision;
			someOutIndices[3 * (i + arrowPrecision) + 1]	= (i + 1) % arrowPrecision;
			someOutIndices[3 * (i + arrowPrecision) + 2]	= (i + 1) % arrowPrecision + arrowPrecision;

			// y
			const uint yOffset = arrowPrecision * 2;
			someOutIndices[3 * (i + yOffset)]						= someOutIndices[3 * i] + yOffset;
			someOutIndices[3 * (i + yOffset) + 1]					= someOutIndices[3 * i + 1] + yOffset;
			someOutIndices[3 * (i + yOffset) + 2]					= someOutIndices[3 * i + 2] + yOffset;
			someOutIndices[3 * (i + yOffset + arrowPrecision)]		= someOutIndices[3 * (i + arrowPrecision)] + yOffset;
			someOutIndices[3 * (i + yOffset + arrowPrecision) + 1]	= someOutIndices[3 * (i + arrowPrecision) + 1] + yOffset;
			someOutIndices[3 * (i + yOffset + arrowPrecision) + 2]	= someOutIndices[3 * (i + arrowPrecision) + 2] + yOffset;

			// z
			const uint zOffset = 2 * arrowPrecision * 2;
			someOutIndices[3 * (i + zOffset)]						= someOutIndices[3 * i] + zOffset;
			someOutIndices[3 * (i + zOffset) + 1]					= someOutIndices[3 * i + 1] + zOffset;
			someOutIndices[3 * (i + zOffset) + 2]					= someOutIndices[3 * i + 2] + zOffset;
			someOutIndices[3 * (i + zOffset + arrowPrecision)]		= someOutIndices[3 * (i + arrowPrecision)] + zOffset;
			someOutIndices[3 * (i + zOffset + arrowPrecision) + 1]	= someOutIndices[3 * (i + arrowPrecision) + 1] + zOffset;
			someOutIndices[3 * (i + zOffset + arrowPrecision) + 2]	= someOutIndices[3 * (i + arrowPrecision) + 2] + zOffset;
		}
	}
#endif
}
