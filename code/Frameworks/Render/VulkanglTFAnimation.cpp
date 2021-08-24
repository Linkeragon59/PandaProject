#include "VulkanglTFAnimation.h"

#include "VulkanglTFModel.h"
#include "VulkanglTFNode.h"

namespace Render::Vulkan::glTF
{
	void Animation::Load(Model* aContainer, const tinygltf::Model& aModel, uint anAnimationIndex)
	{
		const tinygltf::Animation& gltfAnimation = aModel.animations[anAnimationIndex];

		myName = gltfAnimation.name;

		// Samplers
		mySamplers.resize(gltfAnimation.samplers.size());
		for (uint i = 0; i < (uint)gltfAnimation.samplers.size(); ++i)
		{
			const tinygltf::AnimationSampler& gltfSampler = gltfAnimation.samplers[i];

			// Read sampler input time values
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfSampler.input];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				Assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
				{
					mySamplers[i].myInputTimes.push_back(buf[index]);
				}

				for (float inputTime : mySamplers[i].myInputTimes)
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
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				Assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3:
					{
						const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							mySamplers[i].myOutputValues.push_back(glm::vec4(buf[index], 0.0f));
						}
					}
					break;
				case TINYGLTF_TYPE_VEC4:
					{
						const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							mySamplers[i].myOutputValues.push_back(buf[index]);
						}
					}
					break;
				default:
					Assert(false, "AnimationSampler output format not supported yet");
					break;
				}
			}
		}

		// Channels
		myChannels.resize(gltfAnimation.channels.size());
		for (uint i = 0; i < (uint)gltfAnimation.channels.size(); ++i)
		{
			const tinygltf::AnimationChannel& gltfChannel = gltfAnimation.channels[i];

			if (gltfChannel.target_path == "rotation")
				myChannels[i].myPath = AnimationChannel::Path::ROTATION;
			else if (gltfChannel.target_path == "translation")
				myChannels[i].myPath = AnimationChannel::Path::TRANSLATION;
			else if (gltfChannel.target_path == "scale")
				myChannels[i].myPath = AnimationChannel::Path::SCALE;
			else
				Assert(false, "AnimationChannel not supported yet");

			myChannels[i].mySamplerIndex = gltfChannel.sampler;
			myChannels[i].myNode = aContainer->GetNodeByIndex(gltfChannel.target_node);
		}

		myCurrentTime = myStartTime;
	}

	void Animation::Update(float aDeltaTime)
	{
		myCurrentTime += aDeltaTime;
		if (myCurrentTime > myEndTime)
			myCurrentTime -= myEndTime;

		for (AnimationChannel& channel : myChannels)
		{
			AnimationSampler& sampler = mySamplers[channel.mySamplerIndex];
			for (size_t i = 0; i < sampler.myInputTimes.size() - 1; i++)
			{
				if ((myCurrentTime >= sampler.myInputTimes[i]) && (myCurrentTime <= sampler.myInputTimes[i + 1]))
				{
					float u = (myCurrentTime - sampler.myInputTimes[i]) / (sampler.myInputTimes[i + 1] - sampler.myInputTimes[i]);
					Assert(u >= 0.0f && u <= 1.0f);
					
					switch (channel.myPath)
					{
					case AnimationChannel::Path::TRANSLATION:
						channel.myNode->myTranslation = glm::vec3(glm::mix(sampler.myOutputValues[i], sampler.myOutputValues[i + 1], u));
						break;
					case AnimationChannel::Path::ROTATION:
						glm::quat q1;
						q1.x = sampler.myOutputValues[i].x;
						q1.y = sampler.myOutputValues[i].y;
						q1.z = sampler.myOutputValues[i].z;
						q1.w = sampler.myOutputValues[i].w;
						glm::quat q2;
						q2.x = sampler.myOutputValues[i + 1].x;
						q2.y = sampler.myOutputValues[i + 1].y;
						q2.z = sampler.myOutputValues[i + 1].z;
						q2.w = sampler.myOutputValues[i + 1].w;
						channel.myNode->myRotation = glm::normalize(glm::slerp(q1, q2, u));
						break;
					case AnimationChannel::Path::SCALE:
						channel.myNode->myScale = glm::vec3(glm::mix(sampler.myOutputValues[i], sampler.myOutputValues[i + 1], u));
						break;
					default:
						break;
					}
				}
			}
		}
	}
}
