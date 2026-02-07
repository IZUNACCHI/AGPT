#include "ObjectPool.h"
#include "GameObject.h"

void ObjectPool::Release(const std::string& poolKey, const std::shared_ptr<GameObject>& obj) {
	if (!obj) {
		return;
	}
	// Don't pool destroyed/queued objects.
	if (obj->IsDestroyed() || obj->IsMarkedForDestruction()) {
		return;
	}
	obj->SetActive(false);
	m_pool[poolKey].push_back(obj);
}

void ObjectPool::Clear() {
	m_pool.clear();
}
