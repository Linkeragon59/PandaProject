#include "glTFAnimation.h"

#include "glTFNode.h"

namespace Render
{
	void glTFAnimation::Load(const tinygltf::Model& aModel, uint32_t anAnimationIndex)
	{
		const tinygltf::Animation& gltfAnimation = aModel.animations[anAnimationIndex];

		myName = gltfAnimation.name;

		// Samplers
		for (const tinygltf::AnimationSampler& gltfSampler : gltfAnimation.samplers)
		{
			glTFAnimationSampler sampler{};

			// Read sampler input time values
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfSampler.input];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				float* buf = new float[accessor.count];
				memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(float));
				for (size_t index = 0; index < accessor.count; index++)
				{
					sampler.myTimes.push_back(buf[index]);
				}
				delete[] buf;

				for (float inputTime : sampler.myTimes)
				{
					if (inputTime < myStartTime)
						myStartTime = inputTime;
					if (inputTime > myEndTime)
						myEndTime = inputTime;
				}
			}

			// Read sampler output T/R/S values 
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfSampler.output];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3:
				{
					glm::vec3* buf = new glm::vec3[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::vec3));
					for (size_t index = 0; index < accessor.count; index++)
					{
						sampler.myValues.push_back(glm::vec4(buf[index], 0.0f));
					}
					delete[] buf;
					break;
				}
				case TINYGLTF_TYPE_VEC4:
				{
					glm::vec4* buf = new glm::vec4[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::vec4));
					for (size_t index = 0; index < accessor.count; index++)
					{
						sampler.myValues.push_back(buf[index]);
					}
					delete[] buf;
					break;
				}
				default:
					assert(false);
					break;
				}
			}

			mySamplers.push_back(sampler);
		}

		// Channels
		for (const tinygltf::AnimationChannel& gltfChannel : gltfAnimation.channels)
		{
			glTFAnimationChannel channel{};

			if (gltfChannel.target_path == "rotation")
			{
				channel.myType = glTFAnimationChannel::Type::ROTATION;
			}
			if (gltfChannel.target_path == "translation")
			{
				channel.myType = glTFAnimationChannel::Type::TRANSLATION;
			}
			if (gltfChannel.target_path == "scale")
			{
				channel.myType = glTFAnimationChannel::Type::SCALE;
			}
			if (gltfChannel.target_path == "weights")
			{
				assert(false);
				continue;
			}
			channel.mySamplerIndex = gltfChannel.sampler;
			channel.myNodeIndex = gltfChannel.target_node;
			if (!channel.myNodeIndex)
				continue;

			myChannels.push_back(channel);
		}
	}
}
