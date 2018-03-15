/*
1、快速排序：输入数组，开始下标和结束下标
*/
void myqsort(int *a, int m1, int m2){	
	//数组只有一个数的时候，直接返回
	int i=m1, j=m2,temp;
	if (i == j)
		return;
	//挑选哨兵，开始快排
	temp = a[i];
	while (i != j){
		while (a[j] >= temp&&i < j)
			j--;
		if (i < j) a[i] = a[j];
		while (a[i] <= temp&&i < j)
			i++;
		if (i < j) a[j] = a[i];
	}
	//哨兵归位
	a[i] = temp;
	//进行左右递归，并且判断左右是否长度为0，为0则不递归。
	if (i>m1)
		myqsort(a, m1, i - 1);
	if (j < m2)
		myqsort(a, j + 1, m2);
}

/*
重建二叉树：输入某二叉树的前序遍历和中序遍历的结果，请重建出该二叉树。
假设输入的前序遍历和中序遍历的结果中都不含重复的数字。例如输入前序遍
历序列{1,2,4,7,3,5,6,8}和中序遍历序列{4,7,2,1,5,3,8,6}，则重建二叉树
并返回。
*/

TreeNode* reConstructBinaryTree(vector<int> pre, vector<int> vin) {
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	/*递归，函数返回根节点，或者是NULL*/
	if (pre.size() == 0 || vin.size() == 0)
		return NULL;
	root->val = pre[0];
	//计算左右子树的pre,vin
	vector<int> lpre, lvin;
	vector<int> rpre, rvin;
	vector<int>::iterator result = find(vin.begin(), vin.end(), pre[0]); //在中序遍历中找到左子树
	lvin = vector<int>(vin.begin(), result);
	rvin = vector<int>(result + 1, vin.end());
	lpre = vector<int>(pre.begin() + 1, pre.begin() + 1 + lvin.size());
	rpre = vector<int>(pre.begin() + 1 + lvin.size(), pre.end());
	root->left = reConstructBinaryTree(lpre, lvin);
	root->right = reConstructBinaryTree(rpre, rvin);
	return root;
}