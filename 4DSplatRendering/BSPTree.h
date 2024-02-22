#pragma once

#include<functional>
#include<vector>


template<typename T>
class BSPTree
{
private:

	struct BSPNode
	{
		T* mElement = nullptr;
		BSPNode* mLeft = nullptr;
		BSPNode* mRight = nullptr;
		BSPNode* mParent = nullptr;
	};

	std::function<bool(T*, T*)>& mIsBigger;
	BSPNode* root = nullptr;
	BSPNode* mPrealloc = nullptr;
	size_t mNextPtr = 0;

	void AddRecursive(T* element, BSPNode* root);
	void AddLin(T* element, BSPNode* root);
	void DeleteNodesRecursive(BSPNode* root);

	void BackToFrontRec(std::function<void(T* e)>& op, BSPNode* current);
	void FrontToBackRec(std::function<void(T* e)>& op, BSPNode* current);

	BSPNode* GetNew();

public:

	BSPTree(std::function<bool(T*, T*)>& sorter)
		:
		mIsBigger{sorter}
	{
		root = new BSPNode(); //cursed ... i know

	}

	BSPTree(std::function<bool(T*, T*)>& sorter, size_t preAllocSize)
		:
		mIsBigger{ sorter }
	{
		root = new BSPNode(); //cursed ... i know
		mPrealloc = (BSPNode*) malloc(preAllocSize * sizeof(BSPNode));
	}

	BSPTree(std::function<bool(T*, T*)>& sorter, std::vector<T>& elements)
		:
		mIsBigger{ sorter }
	{
		root = new BSPNode(); //cursed ... i know

		Add(elements);

	}

	~BSPTree();

	void Add(T* e);
	void Add(std::vector<T>& e);
	
	void BackToFront(std::function<void(T* e)>& op);
	void FrontToBack(std::function<void(T* e)>& op);

	void DiscardAndRealloc(size_t preAllocSize);

};

template<typename T>
inline void BSPTree<T>::AddRecursive(T* element, BSPNode* root)
{
	if (root == nullptr) return;
	if (root != nullptr && root->mElement == nullptr) 
	{
		root->mElement = element;
		return;
	}
	if (this->mIsBigger(element, root->mElement))
	{
		if (!root->mRight) root->mRight = new BSPNode();
		AddRecursive(element, root->mRight);
	}
	else 
	{
		if (!root->mLeft) root->mLeft = new BSPNode();
		AddRecursive(element, root->mLeft);
	}
}

template<typename T>
inline void BSPTree<T>::AddLin(T* element, BSPNode* root)
{
	if (root == nullptr) return;
	
	BSPNode* prev = root;
	BSPNode* current = root;

	if (!current->mElement) 
	{
		root->mElement = element;
		return;
	}

	while (true) 
	{
		bool res = this->mIsBigger(element, current->mElement);
		prev = current;
		current = res ? current->mRight : current->mLeft;

		if (current == nullptr) 
		{
			current = new BSPNode();
			current->mElement = element;
			current->mParent = prev;
			if (res) 
			{
				prev->mRight = current;
			}
			else 
			{
				prev->mLeft = current;
			}

			return;
		}
				
	}

}

template<typename T>
inline void BSPTree<T>::DeleteNodesRecursive(BSPNode* root)
{
	if (root->mLeft != nullptr) {
		DeleteNodesRecursive(root->mLeft);
	}

	if (root->mRight != nullptr) {
		DeleteNodesRecursive(root->mRight);
	}

	delete root;
}

template<typename T>
inline void BSPTree<T>::BackToFrontRec(std::function<void(T* e)>& op, BSPNode* current)
{
	if (current == nullptr) return;

	if (current->mRight) 
	{
		BackToFrontRec(op, current->mRight);
	}

	op(current->mElement);

	if (current->mLeft) 
	{
		BackToFrontRec(op, current->mLeft);
	}
}

template<typename T>
inline void BSPTree<T>::FrontToBackRec(std::function<void(T* e)>& op, BSPNode* current)
{
	if (current == nullptr) return;

	if (current->mLeft)
	{
		BackToFrontRec(op, current->mLeft);
	}

	op(current->mElement);

	if (current->mRight)
	{
		BackToFrontRec(op, current->mRight);
	}
}

template<typename T>
inline BSPTree<T>::~BSPTree()
{
	DeleteNodesRecursive(this->root);
}

template<typename T>
inline void BSPTree<T>::Add(T* e)
{
	if (root == nullptr) root = new BSPNode();
	//AddRecursive(e, this->root);
	AddLin(e, this->root);
}

template<typename T>
inline void BSPTree<T>::Add(std::vector<T>& e)
{
	if (root == nullptr) root = new BSPNode();
	for (size_t i = 0; i < e.size(); ++i) 
	{
		AddLin(&e[i], this->root);
		//AddRecursive(&e[i], this->root);
	}
}

template<typename T>
inline void BSPTree<T>::BackToFront(std::function<void(T* e)>& op)
{
	BackToFrontRec(op, this->root);
}

template<typename T>
inline void BSPTree<T>::FrontToBack(std::function<void(T* e)>& op)
{
	FrontToBackRec(op, this->root);
}

template<typename T>
inline void BSPTree<T>::DiscardAndRealloc(size_t preAllocSize)
{
	delete mPrealloc;
	mPrealloc = nullptr;
	mNextPtr = 0;
	mPrealloc = (BSPNode*)malloc(preAllocSize * sizeof(BSPNode));
}
