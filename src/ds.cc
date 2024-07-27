template<typename T>
struct DynamicArray {
    T *mem;
    u32 count;
    u32 len;

    void zero(){
	count = 0;
	len = 0;
    }
    void realloc(u32 newCap) {
	void *newMem = alloc(sizeof(T) * newCap);
	memcpy(newMem, mem, sizeof(T) * len);
	afree(mem);
	mem = (T*)newMem;
	len = newCap;
    };
    T &getElement(u32 index) {
#if(DBG == true)
	if (index >= len) {
	    printf("\n[ERROR]: abc(dynamic_array) failed @ index = %d\n", index);
	};
#endif
	return mem[index];
    };
    T &operator[](u32 index) { return getElement(index); };
    void init(u32 startCount = 5) {
	count = 0;
	len = startCount;
	mem = (T*)alloc(sizeof(T) * startCount);
    };
    void uninit() { afree(mem); };
    void push(const T &t) {
	if (count == len) { realloc(len + len / 2 + 1); };
	mem[count] = t;
	count += 1;
    };
    T pop(){
	count -= 1;
	return mem[count];
    };
    T& newElem(){
	if (count == len) { realloc(len + (len/2) + 1); };
	count += 1;
	return mem[count-1];
    };
#if(DBG)
    void dumpStat() {
	printf("\n[DYNAMIC_ARRAY] mem: %p; count: %d; len: %d\n", mem, count, len);
    };
#endif
};
