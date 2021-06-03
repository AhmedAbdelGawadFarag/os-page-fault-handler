#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

uint32 startFreeVrAddress = KERNEL_HEAP_START;
struct allinfo{
	int vraddress;
	unsigned int size;

}vrpinfo[100];
int vrpinfoindx = 0;
void* kmalloc(unsigned int size)
{
		if(!checkInsuffisentSpace(size))return NULL;


		vrpinfo[vrpinfoindx].vraddress = startFreeVrAddress;
		vrpinfo[vrpinfoindx].size = size;
		uint32 *ptrpgtable  = NULL;
		vrpinfoindx++;



	   unsigned int pgnumbers = cielInteg(size,PAGE_SIZE);
	   uint32 temp = startFreeVrAddress;
		for(int i=0;i<pgnumbers;i++,startFreeVrAddress+=4096){
			struct Frame_Info *frameptr = NULL;
			allocate_frame(&frameptr);

			map_frame(ptr_page_directory,frameptr,(void*)startFreeVrAddress,PERM_WRITEABLE);
		}

		return ((void*)temp);
}

void kfree(void* virtual_address)
{

	int pgnumbers = cielInteg(getAllocSize(virtual_address),PAGE_SIZE);

	for(int i= (int)virtual_address,j=0; j<pgnumbers ;j++, i+=4096){
	unmap_frame(ptr_page_directory,(void*)i);
	}

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{

	struct Frame_Info *physframe = to_frame_info(physical_address);
	for(int i=0;i<vrpinfoindx;i++){
		long long temp = getPhysicalFrames(vrpinfo[i].vraddress,vrpinfo[i].size,physical_address,physframe);
		if( temp !=-1){
			return temp;
		}
	}
	return 0;

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32 *ptrpg = NULL;
	struct Frame_Info *frameptr = get_frame_info(ptr_page_directory,(void *)virtual_address,&ptrpg);

	if(frameptr==NULL)return 0;

	uint32 stframeaddr = ROUNDDOWN(virtual_address,4096);
	uint32 phyaddrs = to_physical_address(frameptr);

	return  phyaddrs+(virtual_address-stframeaddr);

}
int cielInteg(int a,int b){
	return (a+b-1)/b;
}
int checkInsuffisentSpace(unsigned int size){
	int frameNumbers = cielInteg(size,PAGE_SIZE);
	int KernalHeapSize = cielInteg(KERNEL_HEAP_MAX-startFreeVrAddress + 1,PAGE_SIZE);

	return (frameNumbers<KernalHeapSize);
}
int getAllocSize(void *vraddrs){
	for(int i=0; i<vrpinfoindx ;i++){
		if((void*)vrpinfo[i].vraddress == vraddrs)return vrpinfo[i].size;
	}
	return -1;//not found
}
long long getPhysicalFrames(uint32 vraddrs,int size,uint32 physical_address,struct Frame_Info *physcptr){
	int pgnumbers = cielInteg(size,PAGE_SIZE);

	for(uint32 i=0,j = vraddrs ;i<pgnumbers;i++,j+=PAGE_SIZE){
		uint32 *ptrpgtable = NULL;
		struct Frame_Info *frameptr = get_frame_info(ptr_page_directory,(void*)j,&ptrpgtable);
		 uint32 tempphyaddrs = to_physical_address(frameptr);
		 if(physcptr == frameptr ){
			 return j+(physical_address-tempphyaddrs);
		 }

	}
	return -1;

}

