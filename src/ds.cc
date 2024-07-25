//array whose length is not known at comptime
template<typename T>
struct Array {
	T *mem;
	u32 len;
	u32 count;
	void init(const u32 length){
		count = 0;
		len = length;
		mem = (T*)alloc(sizeof(T)*len);
	};
	void uninit(){
		afree(mem);
		len = 0;
		count = 0;
	};
	T& operator[](const u32 index) {
#if(DBG)
		if(index >= len) {
			clog("\n[ERROR]: abc(array) failed @ index = %d\n", index);
			return mem[0];
		};
#endif
		return mem[index];
	    };
	void push(const T &t){
#if(DBG)
		if(count >= len){
			clog("\n[ERROR]: push(array) failed as count(%d) >= len(%d)\n", count, len);
			return;
		};
#endif
		mem[count] = t;
		count += 1;
	};
};
