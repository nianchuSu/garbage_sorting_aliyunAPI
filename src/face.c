#include <Python.h>
#include <stdio.h>

#include "face.h"

// 拍照
#define SHOOT "wget http://127.0.0.1:8080/?action=snapshot -O /img/SearchFace.jpg"
// #define SHOOT "wget http://127.0.0.1:8080/?action=snapshot -O ~/smart/image/lj.jpg"
#define PORTRAIT_FILE "/img/SearchFace.jpg"    // 图像相对路径


/*
 *老实的认真读代码
 不懂就百度,chatGPT查
 * */

void py_init(void)	// 初始化python环境
{
	Py_Initialize();	// 初始化解释器
						//
	PyObject *sys = PyImport_ImportModule("sys");		// 当前路径或者需要用的py模块/文件路径导入
	PyObject *path = PyObject_GetAttrString(sys,"path");	// sys导入path列表	获取path引用
	PyList_Append(path, PyUnicode_FromString("."));	// 将当前路径添加到path列表里	将c字符串转化为py字符串
}

void py_finish(void)	// 释放资源都要执行,关闭python环境
{
	Py_Finalize();	// py解释器关闭
}

/*
 *这个程序是无参调用函数,获得返回值,并解析返回值
 *返回值给c使用
 */
double par_retfunc(void)	// 传入一个指针
{
	// 获得函数调用返回值
	double ret=0;
	system(SHOOT);                       // 调用摄像头
    if (0 != access(PORTRAIT_FILE, F_OK)) // 判断图像是否存在
	{
		printf("没照片\n");
		return ret;	// 不存在,直接返回
	}
	PyObject *pModule = PyImport_ImportModule("face");	// 导入模块,获取对应模块的引用,参数就是使用的模块名		对应同级目录下的py文件名
	
	if(!pModule){	// 为NULL就进入
		PyErr_Print();	// 调用py异常报错
		printf("error Module\n");
		goto f_pmodu;
	}

	PyObject *pFunc = PyObject_GetAttrString(pModule,"aliyun_face");	// 获取模块中已有的函数名称如face模块中的pr函数的对象与引用,并传入C字符串参数函数名		py文件的函数名
	if(!pFunc){	// 为NULL就进入
		PyErr_Print();	// 调用py异常报错
		printf("error Func\n");
		goto f_pfunc;
	}


	PyObject *pValue = PyObject_CallObject(pFunc,NULL);	// 调用函数获取返回值,参数1函数引用参数2是函数参数元组,调用函数
	if(!pValue){	// 为NULL就进入
		PyErr_Print();	// 调用py异常报错
		printf("error Value\n");
		goto f_pval;
	}
	
	if(!PyArg_Parse(pValue,"d",&ret)){	// 这个函数是取得函数调用返回值,存入ret中,让C使用,这里需要注意返回值类型与承接变量类型一致,"i"表示整数型,其他类型,网上找相关资料	解释函数的返回值
		PyErr_Print();	// 调用py异常报错
		printf("error return\n");
		exit(-1);
	}

	printf("return:%lf\n",ret);

f_pval:		// goto语句,百度查
	Py_DECREF(pValue);	// 关闭所有对象,依次向上关闭
f_pfunc:
	Py_DECREF(pFunc);	// 关闭所有对象,依次向上关闭
f_pmodu:
	Py_DECREF(pModule);	// 关闭所有对象,依次向上关闭


	return ret;
}