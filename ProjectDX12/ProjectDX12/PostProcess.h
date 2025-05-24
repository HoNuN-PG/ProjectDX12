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
	T* AddVolume()
	{
		T* volume = new T();
		Volumes.push_back(volume);
		Volumes.back()->Init();
		return volume;
	}

	template <typename T>
	T* GetVolume()
	{
		for (Volume* volume : Volumes)
		{
			if (typeid(*volume) == typeid(T)) // 型を調べる(RTTI動的型情報)
			{
				return (T*)volume;
			}
		}
		return nullptr;
	}

private:
	std::list<Volume*> Volumes;

};

#endif
