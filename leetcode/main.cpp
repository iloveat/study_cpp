#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <string.h>
#include <stack>
#include <set>
#include <unordered_map>

using namespace std;

std::vector<int> twoSum(std::vector<int>& nums, int target)
{
    std::map<int, int> mm;
    std::vector<int> ret;
    for(int i = 0; i < nums.size(); i++)
    {
        int curr = nums[i];
        if(mm.find(curr) == mm.end())
        {
            int rest = target-curr;
            mm[rest] = i;
        }
        else
        {
            ret.push_back(mm[curr]);
            ret.push_back(i);
            cout<<mm[curr]<<","<<i<<endl;
            return ret;
        }
    }
}

int reverse_positive(int x)
{
    long long ret = 0;
    while(x != 0)
    {
        int l = x % 10;
        ret = ret * 10 + l;
        if((int)ret != ret)
            return 0;
        x = x / 10;
    }
    return ret;
}

int reverse(int x) {
    if(x == 0)
        return 0;
    int sign = 1;
    if(x < 0)
        sign = -1;
    int rvs = sign * reverse_positive(sign*x);
    return rvs;
}

bool isPalindrome(int x) {
    if(x==0)
        return true;
    else if(x < 0 || x % 10 == 0)
        return false;
    else
    {
        int sum = 0;
        while(x > sum)
        {
            int l = x % 10;
            sum = sum * 10 + l;
            x = x / 10;
        }
        return (x==sum/10) || (x==sum);
    }
}

std::string longestCommonPrefix(std::vector<std::string>& strs)
{
    std::string ret = "";
    if(strs.size() > 0)
    {
        unsigned int len = strs[0].size();
        for(unsigned int i = 0; i < len; i++)
        {
            char curr = strs[0][i];
            bool bError = false;
            for(unsigned int j = 1; j < strs.size(); j++)
            {
                if(i > strs[j].size()-1 || strs[j][i] != curr)
                {
                    bError = true;
                    break;
                }
            }

            if(bError)
            {
                break;
            }
            else
            {
                ret += curr;
            }
        }
    }
    return ret;
}

bool isClosed(char left, char right)
{
    return (left=='(' && right==')') || (left=='[' && right==']') || (left=='{' && right=='}');
}

bool isValid_slow(std::string s) {
    if(s.size()==0 || s.size()%2==1)
    {
        return false;
    }
    std::vector<char> stk;
    stk.push_back(s[0]);
    for(unsigned int i = 1; i < s.size(); i++)
    {
        char c = s[i];
        if(stk.empty())
        {
            if(c==')' || c==']' || c=='}')
            {
                return false;
            }
        }
        if(isClosed(stk.back(), c))
        {
            stk.pop_back();
        }
        else
        {
            stk.push_back(c);
        }
    }
    return stk.empty();
}

bool isValid(std::string s) {
    if(s.size()==0 || s.size()%2==1)
    {
        return false;
    }
    std::vector<char> stk;
    for(unsigned int i = 0; i < s.size(); i++)
    {
        char c = s[i];
        if(c==')' || c==']' || c=='}')
        {
            if(stk.empty())
            {
                return false;
            }
            else if(isClosed(stk.back(), c))
            {
                stk.pop_back();
            }
            else
            {
                return false;
            }
        }
        else
        {
            stk.push_back(c);
        }

    }
    return stk.empty();
}

/*
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(NULL) {}
};

ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
    if(l1 == NULL) {
        return l2;
    } else if(l2 == NULL) {
        return l1;
    } else {
        ListNode *p1 = l1;
        ListNode *p2 = l2;
        ListNode *qq = NULL;
        ListNode *head = NULL;
        while(p1 != NULL && p2 != NULL) {
            if(p1->val < p2->val) {
                ListNode *cur = new ListNode(p1->val);
                if(head == NULL) {
                    head = cur;
                    qq = cur;
                } else {
                    qq->next = cur;
                    qq = qq->next;
                }
                p1 = p1->next;
            } else {
                ListNode *cur = new ListNode(p2->val);
                if(head == NULL) {
                    head = cur;
                    qq = cur;
                } else {
                    qq->next = cur;
                    qq = qq->next;
                }
                p2 = p2->next;
            }
        }
        if(p1 == NULL) {
            while(p2) {
                ListNode *cur = new ListNode(p2->val);
                qq->next = cur;
                qq = qq->next;
                p2 = p2->next;
            }
        } else {
            while(p1) {
                ListNode *cur = new ListNode(p1->val);
                qq->next = cur;
                qq = qq->next;
                p1 = p1->next;
            }
        }
        return head;
    }
}

ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
    if(l1 == NULL) return l2;
    if(l2 == NULL) return l1;
    if(l1->val < l2->val) {
        l1->next = mergeTwoLists(l1->next, l2);
        return l1;
    } else {
        l2->next = mergeTwoLists(l1, l2->next);
        return l2;
    }
}
*/

int removeDuplicates(std::vector<int>& nums) {
    int size = nums.size();
    if(size <= 1) return size;
    int i = 0;
    int j = 1;
    int ret;
    while(i < size) {
        while(j < size && nums[i] == nums[j]) {
            j++;
        }
        if(j == size) {
            ret = i + 1;
            break;
        } else {
            i++;
            nums[i] = nums[j];
            j++;
        }
    }
    for(int i = 0; i < ret; i++) {
        cout<<nums[i]<<",";
    }
    cout<<endl;
    return ret;
}

/*
int removeElement(std::vector<int>& nums, int val) {
    int size = nums.size();
    if(size == 0) {
        return size;
    }
    int i = 0;
    int j = 0;
    int ret = 0;
    while(i < size) {
        while(i < size && nums[i] == val) {
            i++;
        }
        if(i == size) {
            ret = j + 1;
            break;
        } else {
            nums[j] = nums[i];
            j++;
            i++;
        }
    }
    if(i == size) {
        ret = j;
    }
    for(int i = 0; i < ret; i++) {
        cout<<nums[i]<<",";
    }
    cout<<endl;
    return ret;
}
*/

int removeElement(std::vector<int>& nums, int val) {
    for(std::vector<int>::iterator it = nums.begin(); it != nums.end(); ) {
        if(*it == val) {
            it = nums.erase(it);
        } else {
            it++;
        }
    }
    for(int i = 0; i < nums.size(); i++) {
        cout<<nums[i]<<",";
    }
    cout<<endl;
    return nums.size();
}

int strStr(std::string haystack, std::string needle) {
    int steps = haystack.size()-needle.size()+1;
    int len = needle.size();
    for(int i = 0; i < steps; i++) {
        if(haystack.substr(i, len) == needle) {
            return i;
        }
    }
    return -1;
}

int searchInsert_old(std::vector<int>& nums, int target) {
    int len = nums.size();
    int idx = -1;
    for(int i = 0; i < len; i++) {
        if(nums[i] == target) {
            return i;
        } else {
            if(nums[i] > target) {
                idx = i;
                break;
            }
        }
    }
    if(idx == -1) {
        return len;
    } else if(idx == 0) {
        return 0;
    } else {
        return idx;
    }
}

int searchInsert(std::vector<int>& nums, int target) {
    int l = 0;
    int r = nums.size()-1;
    while(l <= r) {
        int m = (l+r) >> 1;
        if(nums[m] == target) {
            return m;
        } else if(nums[m] < target) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }
    return l;
}

std::string countAndSay_old(int n) {
    if(n == 1) return "1";
    if(n == 2) return "11";
    std::string str = countAndSay_old(n-1);
    int counter = 1;
    std::string result = "";
    for(int i = 0; i < str.size(); i++) {
        int value = str[i]-'0';
        if(i < str.size()-1) {
            if(str[i] == str[i+1]) {
                counter++;
            } else {
                result += '0'+counter;
                result += '0'+value;
                counter = 1;
            }
        } else {
            result += '0'+counter;
            result += '0'+value;
        }
    }
    return result;
}

std::string countAndSay(int n) {
    if(n == 1) return "1";
    if(n == 2) return "11";
    std::string str = countAndSay(n-1);
    std::string result = "";
    int counter = 1;
    int value = str[0] - '0';
    for(int i = 1; i < str.size(); i++) {
        if(str[i] == str[i-1]) {
            counter++;
        } else {
            result.append(to_string(counter));
            result.append(to_string(value));
            value = str[i] - '0';
            counter = 1;
        }
        if(i == str.size()-1) {
            result.append(to_string(counter));
            result.append(to_string(value));
        }
    }
    return result;
}

int maxSubArray(std::vector<int>& nums) {
    int sum = nums[0];
    int max_sum = nums[0];
    for(int i = 1; i < nums.size(); i++) {
        int cur = nums[i];
        if(sum < 0) {
            sum = cur;
        } else {
            sum += cur;
        }
        if(sum > max_sum) {
            max_sum = sum;
        }
    }
    return max_sum;
}

int lengthOfLastWord(std::string s) {
    int counter = 0;
    bool bStart = false;
    for(int i = s.size()-1; i >= 0; i--) {
        if(bStart) {
            if(s[i] == ' ') {
                break;
            } else {
                counter++;
            }
        } else {
            if(s[i] != ' ') {
                bStart = true;
                counter++;
            }
        }
    }
    return counter;
}

std::vector<int> plusOne(std::vector<int>& digits) {
    std::vector<int> result = digits;
    int inc = 1;
    for(int i = result.size()-1; i >= 0; i--) {
        if(result[i] + inc > 9) {
            result[i] = result[i] + inc - 10;
            inc = 1;
        } else {
            result[i] = result[i] + inc;
            inc = 0;
        }
    }
    if(inc == 1) {
        result.insert(result.begin(), 1);
    }
    return result;
}

std::string addBinary(std::string a, std::string b) {
    int la = a.size();
    int lb = b.size();
    int ll = la > lb ? la : lb;
    if(la > lb) {
        std::string s(la-lb, '0');
        b = s + b;
    } else if(la < lb) {
        std::string s(lb-la, '0');
        a = s + a;
    }
    std::string result(ll, '0');
    int inc = 0;
    for(int i = ll-1; i >= 0; i--) {
        int na = a[i] - '0';
        int nb = b[i] - '0';
        if(na + nb + inc > 1) {
            result[i] = '0' + na + nb + inc - 2;
            inc = 1;
        } else {
            result[i] = '0' + na + nb + inc;
            inc = 0;
        }
    }
    if(inc == 1) {
        result.insert(result.begin(), '1');
    }
    return result;
}

int mySqrt_old(int x) {
    if(x == 0) return 0;
    double x1 = 1.0;
    double x2 = (x1 + x / x1) / 2.0;
    while(fabs(x1-x2) > 1e-1) {
        x1 = x2;
        x2 = (x1 + x / x1) / 2.0;
    }
    return (int)x2;
}

int mySqrt(int x) {
    if(x < 1) return 0;
    if(x < 4) return 1;
    int l = 1;
    int r = x / 2;
    while(l <= r) {
        long m = (l+r) / 2;
        if(m * m == x) {
            return m;
        } else if(m * m < x) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }
    return r;
}

int climbStairs_old(int n) {
    static vector<int> lut(n, 0);
    if(n == 1) return 1;
    if(n == 2) return 2;
    if(lut[n-1] == 0) {
        lut[n-1] = climbStairs_old(n-1);
    }
    if(lut[n-2] == 0) {
        lut[n-2] = climbStairs_old(n-2);
    }
    return lut[n-1] + lut[n-2];
}

int climbStairs(int n) {
    if(n == 1) return 1;
    if(n == 2) return 2;
    int x2 = 1;
    int x1 = 2;
    for(int i = 2; i < n; i++) {
        int tt = x1;
        x1 = x2 + x1;
        x2 = tt;
    }
    return x1;
}

/*
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(NULL) {}
};

// Given 1->1->2->3->3, return 1->2->3
ListNode* deleteDuplicates(ListNode* head) {
    if(head == NULL) return NULL;
    if(head->next == NULL) return head;
    ListNode *ppre = head;
    ListNode *pcur = head->next;
    while(pcur != NULL) {
        if(pcur->val == ppre->val) {
            ListNode *p = pcur;
            ppre->next = pcur->next;
            pcur = pcur->next;
            delete p;

        } else {
            pcur = pcur->next;
            ppre = ppre->next;
        }
    }
    return head;
}
*/

void merge_old(std::vector<int>& nums1, int m, std::vector<int>& nums2, int n) {
    if(m <= 0) {
        nums1 = nums2;
        return;
    }
    if(n <= 0) {
        return;
    }
    std::vector<int> ret(m+n);
    int i = 0;
    int j = 0;
    int k = 0;
    while(i < m && j < n) {
        if(nums1[i] < nums2[j]) {
            ret[k++] = nums1[i++];
        } else {
            ret[k++] = nums2[j++];
        }
    }
    if(i == m) {
        while(j < n) {
            ret[k++] = nums2[j++];
        }
    } else if(j == n) {
        while(i < m) {
            ret[k++] = nums1[i++];
        }
    }
    nums1 = ret;
}

void merge(std::vector<int>& nums1, int m, std::vector<int>& nums2, int n) {
    if(m <= 0) {
        nums1 = nums2;
        return;
    }
    if(n <= 0) {
        return;
    }
    //nums1.resize(m+n);
    int i = m-1;
    int j = n-1;
    int k = m+n-1;
    while(i >= 0 && j >= 0) {
        if(nums1[i] > nums2[j]) {
            nums1[k--] = nums1[i--];
        } else {
            nums1[k--] = nums2[j--];
        }
    }
    if(i < 0) {
        while(j >= 0) {
            nums1[k--] = nums2[j--];
        }
    }
}

/*
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

bool isSameTree(TreeNode* p, TreeNode* q) {
    if(p == NULL) {
        if(q == NULL) {
            return true;
        } else {
            return false;
        }
    }
    if(q == NULL) {
        return false;
    }
    if(p->val != q->val) {
        return false;
    } else {
        return isSameTree(p->left, q->left) && isSameTree(p->right, q->right);
    }
}

bool isSymmetricTrees(TreeNode *p, TreeNode *q) {
    if(p == NULL) {
        if(q == NULL) {
            return true;
        } else {
            return false;
        }
    }
    if(q == NULL) {
        return false;
    }
    if(p->val != q->val) {
        return false;
    } else {
        return isSymmetricTrees(p->left, q->right) && isSymmetricTrees(p->right, q->left);
    }
}

bool isSymmetric(TreeNode* root) {
    if(root == NULL) {
        return true;
    }
    return isSymmetricTrees(root->left, root->right);
}

int maxDepth(TreeNode* root) {
    if(root == NULL) {
        return 0;
    }
    int dl = maxDepth(root->left);
    int dr = maxDepth(root->right);
    return 1 + (dl > dr ? dl : dr);
}

int maxDepth(TreeNode* root) {
    if(root == NULL) {
        return 0;
    }

    std::queue<pair<TreeNode*, int>> q;
    q.push(make_pair(root, 1));
    int max_depth = 0;

    while(!q.empty()) {
        pair<TreeNode*, int> node = q.front();
        q.pop();
        TreeNode *p = node.first;
        int depth = node.second;
        if(depth > max_depth) {
            max_depth = depth;
        }
        TreeNode *l = p->left;
        TreeNode *r = p->right;
        if(l != NULL) {
            q.push(make_pair(l, depth+1));
        }
        if(r != NULL) {
            q.push(make_pair(r, depth+1));
        }
    }

    return max_depth;
}

std::vector<std::vector<int>> levelOrderBottom(TreeNode* root) {
    if(root == NULL) {
        return std::vector<std::vector<int>>();
    }

    std::vector<pair<TreeNode*, int>> lst;
    std::queue<pair<TreeNode*, int>> q;
    q.push(make_pair(root, 0));

    while(!q.empty()) {
        pair<TreeNode*, int> node = q.front();
        q.pop();
        TreeNode *p = node.first;
        int depth = node.second;
        TreeNode *l = p->left;
        TreeNode *r = p->right;
        if(l != NULL) {
            q.push(make_pair(l, depth+1));
        }
        if(r != NULL) {
            q.push(make_pair(r, depth+1));
        }
        lst.push_back(node);
    }

    pair<TreeNode*, int> bottom = lst[lst.size()-1];
    int max_depth = 1 + bottom.second;
    std::vector<std::vector<int>> result(max_depth);

    for(unsigned int i = 0; i < lst.size(); i++) {
        pair<TreeNode*, int> node = lst[i];
        int val = (node.first)->val;
        int idx = node.second;
        result[max_depth-1-idx].push_back(val);
    }

    return result;
}

TreeNode* sortedArrayToBST(std::vector<int>& nums, int left, int right) {
    if(left > right) {
        return NULL;
    }
    int length = right - left + 1;
    int mid = left + length / 2;
    TreeNode *node = new TreeNode(nums[mid]);
    node->left = sortedArrayToBST(nums, left, mid-1);
    node->right = sortedArrayToBST(nums, mid+1, right);
    return node;
}

TreeNode* sortedArrayToBST(std::vector<int>& nums) {
    int length = nums.size();
    if(length == 0) {
        return NULL;
    }
    return sortedArrayToBST(nums, 0, length-1);
}

int treeHeight(TreeNode *root) {
    if(root == NULL) {
        return 0;
    }
    int lh = treeHeight(root->left);
    int rh = treeHeight(root->right);
    return 1 + (lh > rh ? lh : rh);
}

bool isBalanced(TreeNode* root) {
    if(root == NULL) {
        return true;
    }
    int lh = treeHeight(root->left);
    int rh = treeHeight(root->right);
    if(abs(lh-rh) > 1) {
        return false;
    }
    return isBalanced(root->left) && isBalanced(root->right);
}

int minDepth(TreeNode* root) {
    if(root == NULL) {
        return 0;
    }
    if(root->left == NULL) {
        return 1 + minDepth(root->right);
    }
    if(root->right == NULL) {
        return 1 + minDepth(root->left);
    }
    int lh = minDepth(root->left);
    int rh = minDepth(root->right);
    return 1 + (lh < rh ? lh : rh);
}

bool hasPathSum(TreeNode* root, int sum) {
    if(root == NULL) {
        return false;
    }
    int val = root->val;
    if(root->left == NULL && root->right == NULL) {
        if(sum == val) {
            return true;
        } else {
            return false;
        }
    }
    return hasPathSum(root->left, sum-val) || hasPathSum(root->right, sum-val);
}

bool hasCycle(ListNode *head) {
    if(head == NULL) {
        return false;
    }
    ListNode *slow = head->next;
    if(slow == NULL) {
        return false;
    }
    ListNode *fast = slow->next;
    while(fast != slow) {
        if(fas
            return false;
        }
        slow = slow->next;
        fast = fast->next;
        if(fast == NULL) {
            return false;
        }
        fast = fast->next;
    }
    return true;
}

ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
    if(headA == NULL || headB == NULL) {
        return NULL;
    }
    ListNode *pA = headA;
    ListNode *pB = headB;
    int lenA = 1;
    int lenB = 1;
    while(pA->next) {
        lenA++;
        pA = pA->next;
    }
    while(pB->next) {
        lenB++;
        pB = pB->next;
    }
    if(pA != pB) {
        return NULL;
    }
    pA = headA;
    pB = headB;
    if(lenA > lenB) {
        int n = lenA - lenB;
        for(int i = 0; i < n; i++) {
            pA = pA->next;
        }
    } else if(lenB > lenA) {
        int n = lenB - lenA;
        for(int i = 0; i < n; i++) {
            pB = pB->next;
        }
    }
    while(pA != NULL) {
        if(pA == pB) {
            return pA;
        }
        pA = pA->next;
        pB = pB->next;
    }
}

ListNode* removeElements(ListNode* head, int val) {
    while(head != NULL && head->val == val) {
        ListNode *p = head;
        head = head->next;
        delete p;
    }
    if(head == NULL) {
        return NULL;
    }

    ListNode *pre = head;
    ListNode *cur = head->next;
    while(cur != NULL) {
        if(cur->val == val) {
            ListNode *p = cur;
            pre->next = cur->next;
            cur = cur->next;
            delete p;
        } else {
            pre = pre->next;
            cur = cur->next;
        }
    }
    return head;
}

ListNode* reverseList(ListNode* head) {
    if(head == NULL || head->next == NULL) {
        return head;
    }
    ListNode *pTail = head;
    ListNode *pHead = head->next;
    pTail->next = NULL;
    while(pHead != NULL) {
        ListNode *pNext = pHead->next;
        if(pNext == NULL) {
            pHead->next = pTail;
            return pHead;
        }
        pHead->next = pTail;
        pTail = pHead;
        pHead = pNext;
    }
}

ListNode* reverseList(ListNode* head) {
    if(head == NULL) {
        return NULL;
    }
    std::stack<ListNode*> st;
    while(head != NULL) {
        st.push(head);
        head = head->next;
    }

    head = st.top();
    st.pop();

    ListNode *p = head;
    while(!st.empty()) {
        p->next = st.top();
        st.pop();
        p = p->next;
    }
    p->next = NULL;
    return head;
}

TreeNode* invertTree(TreeNode* root) {
    if(root == NULL) {
        return NULL;
    }
    TreeNode *left = root->left;
    TreeNode *right = root->right;
    root->left = invertTree(right);
    root->right = invertTree(left);
    return root;
}
*/

int singleNumber(std::vector<int>& nums) {
    int ret = 0;
    for(int n : nums) {
        ret ^= n;
    }
    return ret;
}

class MinStack {
public:
    std::stack<int> m_dat_stack;
    std::stack<int> m_min_stack;

    MinStack() {

    }

    void push(int x) {
        m_dat_stack.push(x);
        if(m_min_stack.empty()) {
            m_min_stack.push(x);
        } else {
            int m = m_min_stack.top();
            m_min_stack.push(x < m ? x : m);
        }
    }

    void pop() {
        if(m_dat_stack.empty()) {
            return;
        }
        m_dat_stack.pop();
        m_min_stack.pop();
    }

    int top() {
        int val = m_dat_stack.top();
        return val;
    }

    int getMin() {
        int val = m_min_stack.top();
        return val;
    }
};


int calcuHappy(int n) {
    int ret = 0;
    while(n != 0) {
        int l = n % 10;
        n = n / 10;
        ret = ret + l * l;
    }
    return ret;
}

bool isHappy(int n) {
    std::map<int, int> mp;
    while(true) {
        n = calcuHappy(n);
        if(n == 1) {
            return true;
        } else {
            if(mp.find(n) == mp.end()) {
                mp[n] == 1;
            } else {
                return false;
            }
        }
    }
}

bool isPrime(int n) {
    if(n < 2) {
        return false;
    }
    int m = sqrt(n);
    for(int i = 2; i <= m; i++) {
        if(n % i == 0) {
            return false;
        }
    }
    return true;
}

int countPrimes_slow(int n) {
    if(n <= 2) {
        return 0;
    }
    int ret = 0;
    for(int i = 2; i < n; i++) {
        if(isPrime(i)) {
            ret++;
        }
    }
    return ret;
}

int countPrimes(int n) {
    if(n < 2) {
        return 0;
    }
    std::vector<bool> isprime(n, true);
    int ret = 0;
    for(int i = 2; i < n; i++) {
        if(isprime[i]) {
            ret++;
        }
        for(int j = 2; j < n; j++) {
            int k = i * j;
            if(k >= n) {
                break;
            } else if(isprime[k]) {
                isprime[k] = false;
            }
        }
    }
    return ret;
}

bool isIsomorphic1(std::string s, std::string t) {
    unsigned int len1 = s.length();
    unsigned int len2 = t.length();
    if(len1 != len2) {
        return false;
    }
    std::vector<int> vs(256, -1);  //s->t
    std::vector<int> vt(256, -1);  //t->s
    for(unsigned int i = 0; i < len1; i++) {
        if(vs[s[i]] == -1) {
            vs[s[i]] = t[i];
        } else {
            if(vs[s[i]] != t[i]) {
                return false;
            }
        }
        if(vt[t[i]] == -1) {
            vt[t[i]] = s[i];
        } else {
            if(vt[t[i]] != s[i]) {
                return false;
            }
        }
    }
    return true;
}

bool isIsomorphic2(std::string s, std::string t) {
    unsigned int len1 = s.length();
    unsigned int len2 = t.length();
    if(len1 != len2) {
        return false;
    }
    int ms[256] = { 0 };  //这种方式只能初始化为全零
    int mt[256] = { 0 };
    for(unsigned int i = 0; i < len1; i++) {
        if(ms[s[i]] != mt[t[i]]) {
            return false;
        }
        ms[s[i]] = i+1;
        mt[t[i]] = i+1;
    }
    return true;
}

bool containsDuplicate1(std::vector<int>& nums) {
    std::unordered_map<int, bool> mp;
    for(std::vector<int>::iterator it = nums.begin(); it != nums.end(); it++) {
        if(mp.find(*it) == mp.end()) {
            mp[*it] = 1;
        } else {
            return true;
        }
    }
    return false;
}

bool containsDuplicate2(std::vector<int>& nums) {
    std::set<int> st;
    for(std::vector<int>::iterator it = nums.begin(); it != nums.end(); it++) {
        if(st.count(*it) == 0) {
            st.insert(*it);
        } else {
            return true;
        }
    }
    return false;
}

bool containsNearbyDuplicate(std::vector<int>& nums, int k) {
    std::unordered_map<int, int> mp;
    for(unsigned int i = 0; i < nums.size(); i++) {
        int val = nums[i];
        if(mp.find(val) == mp.end()) {
            mp[val] = i;
        } else {
            int idx = mp[val];
            if(i - idx <= k) {
                return true;
            } else {
                mp[val] = i;
            }
        }
    }
    return false;
}


bool isPowerOfTwo(int n) {
    if(n <= 0) {
        return false;
    }
    int count = 0;
    int width = sizeof(int) * 8;
    for(int i = 0; i < width; i++) {
        if(n & 0x01 == 1) {
            count++;
            if(count > 1) {
                return false;
            }
        }
        n  = n >> 1;
    }
    return count == 1;
}

bool isPowerOfTwo_fast(int n) {
    if(n <= 0) {
        return false;
    }
    return (n & (n-1)) == 0;
}

class MyQueue {
public:
    std::stack<int> mS1;
    std::stack<int> mS2;

    /** Initialize your data structure here. */
    MyQueue() {

    }

    /** Push element x to the back of queue. */
    void push(int x) {
        mS1.push(x);
    }

    /** Removes the element from in front of queue and returns that element. */
    int pop() {
        if(mS2.empty()) {
            while(!mS1.empty()) {
                int val = mS1.top();
                mS1.pop();
                mS2.push(val);
            }
        }
        int ret = mS2.top();
        mS2.pop();
        return ret;
    }

    /** Get the front element. */
    int peek() {
        if(mS2.empty()) {
            while(!mS1.empty()) {
                int val = mS1.top();
                mS1.pop();
                mS2.push(val);
            }
        }
        int ret = mS2.top();
        return ret;
    }

    /** Returns whether the queue is empty. */
    bool empty() {
        return mS1.empty() && mS2.empty();
    }
};


int main()
{
    cout << "Hello World!" << endl;
    std::vector<int> nums = {3, 2, 4};
    int target = 6;
    twoSum(nums, target);

    cout<<reverse(123)<<endl;
    cout<<reverse(-12345)<<endl;
    cout<<reverse(1)<<endl;
    cout<<reverse(1056389759)<<endl;

    std::vector<std::string> strs = {"zoucheng", "zou", "zoudheng"};
    cout<<longestCommonPrefix(strs)<<endl;

    cout<<isValid("[")<<endl;
    cout<<isValid("[()]")<<endl;
    cout<<isValid("([)]")<<endl;
    cout<<isValid("()[]{}")<<endl;

    nums = {1, 1, 3};
    cout<<removeDuplicates(nums)<<endl;

    nums = {3, 2, 2, 3};
    cout<<removeElement(nums, 3)<<endl;

    cout<<strStr("zoucheng", "hen")<<endl;
    cout<<strStr("", "")<<endl;

    nums = {1, 3, 5, 6};
    cout<<searchInsert(nums, 5)<<endl;
    cout<<searchInsert(nums, 2)<<endl;
    cout<<searchInsert(nums, 7)<<endl;
    cout<<searchInsert(nums, 0)<<endl;

    cout<<countAndSay(1)<<endl;
    cout<<countAndSay(2)<<endl;
    cout<<countAndSay(3)<<endl;
    cout<<countAndSay(4)<<endl;
    cout<<countAndSay(5)<<endl;

    nums = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    cout<<maxSubArray(nums)<<endl;
    cout<<lengthOfLastWord("Hello World")<<endl;
    cout<<lengthOfLastWord("Hello World    ")<<endl;
    cout<<lengthOfLastWord("Hello")<<endl;

    nums = {9, 9, 9};
    plusOne(nums);

    cout<<addBinary("0", "1")<<endl;
    cout<<addBinary("1", "1")<<endl;
    cout<<addBinary("1111", "111")<<endl;

    cout<<mySqrt(0)<<endl;
    cout<<mySqrt(1)<<endl;
    cout<<mySqrt(2)<<endl;
    cout<<mySqrt(26)<<endl;
    cout<<mySqrt(2147395599)<<endl;

    cout<<climbStairs(1)<<endl;
    cout<<climbStairs(2)<<endl;
    cout<<climbStairs(3)<<endl;
    cout<<climbStairs(4)<<endl;

    std::vector<int> nums1 = {3,4,7,0,0};
    std::vector<int> nums2 = {1,5};
    merge(nums1, 3, nums2, 2);
    for(unsigned int i = 0; i < nums1.size(); i++) {
        cout<<nums1[i]<<",";
    }
    cout<<endl;

    char str[] = "6,48,2,36,2,48,2,94,4,59,4,48,2,18,4,48,2,64,4,1,2,85,6,48,4,48,";
    const char *splitter = ",";
    char *left = NULL;

    char *p = strtok_r(str, splitter, &left);
    while(p != NULL)
    {
        cout<<p<<";";
        p = strtok_r(NULL, splitter, &left);
    }
    cout<<endl;

    cout<<countPrimes(10)<<endl;
    cout<<isIsomorphic1("foo", "egg")<<endl;
    cout<<isIsomorphic2("aa", "ab")<<endl;

    return 0;
}

