#ifndef ___POST_PROCESS_H___
#define ___POST_PROCESS_H___

#include <list>
#include <memory>

#include "System/Rendering/Volume/volume.h"

/**
 * @class PostProcess
 * @brief ポストプロセス群
 */
class PostProcess
{
public:

	explicit PostProcess(MeshBuffer& screen) : pScreen(&screen) {};
	~PostProcess();
	void Update();
	void Draw();

public:

	template <typename T> 
	T* AddVolume()
	{
		std::unique_ptr<T> volume = std::make_unique<T>();
		T* ptr = volume.get();
		ptr->SetScreen(*pScreen);
		ptr->Init();
		Volumes.push_back(std::move(volume));
		return ptr;
	}
	template <typename T>
	T* GetVolume()
	{
		for (const std::unique_ptr<Volume>& volume : Volumes)
		{
			if (T* v = dynamic_cast<T*>(volume.get()))
			{
				return v;
			}
		}
		return nullptr;
	}

private:

	MeshBuffer* pScreen; // 全画面描画用の四角形（RenderingEngineが所有しPostProcessより長生きする）
	std::list<std::unique_ptr<Volume>> Volumes;

};

#endif
