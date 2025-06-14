#ifndef ___POST_PROCESS_H___
#define ___POST_PROCESS_H___

#include <list>
#include <memory>

#include "volume.h"

class PostProcess
{
public:

	PostProcess() {};
	~PostProcess();
	void Update();
	void Draw();

	template <typename T> // テンプレート関数
	std::shared_ptr<T> AddVolume()
	{
		std::shared_ptr<T> volume = std::make_shared<T>();
		Volumes.push_back(volume);
		Volumes.back()->Init();
		return volume;
	}

	template <typename T>
	std::shared_ptr<T> GetVolume()
	{
		for (std::shared_ptr<Volume> volume : Volumes)
		{
			if (std::shared_ptr<T> v = std::dynamic_pointer_cast<T>(volume))
			{
				return v;
			}
		}
		return nullptr;
	}

private:
	std::list<std::shared_ptr<Volume>> Volumes;

};

#endif
