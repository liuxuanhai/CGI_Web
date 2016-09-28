python CGI use caution!!!

===================================
1.check json and protobuf for python already install on your machine.
  you can use python cmd to check:
  >>>import demjson
  >>>import google.protobuf.internal

2.how to generate msg_cgi/proto/*.py ?
  generate *.py in /src/common/proto use cmd:
  
  protoc --python_out=./ ./*.proto
  mv *py /src/cgi_python/common/proto/

3.remind chmod a+x for executable python script
  such as : 
  -rwxr-xr-x. 1 douyu douyu 2722 Apr 28 18:49 cgi_msg_add.py

4.string -> uint64_t in python, you can only use int() function
  "python 自带大数整数运算，整数不会溢出，只要内存足够, 3.x后不再区分int/long,去除了long,统称int"
  
  example:
  >>>openid_md5 = int('13594908550356151498') 
  >>>print 'openid_md5=%d' % openid_md5

5.google protocol buffer在python中不支持utf-8编码的字符串只支持unicode编码的字符串，
  所以给protobuf中字符串赋值时，要将字符串由utf-8 -> unicode，尤其是中文字符：

  example:
  >>>strMsgContent = '中文测试！'
  >>>msgcontent.content = strMsgContent.decode('utf-8')

6.decode('unicode-escape'), 将反斜杠u类型 '\uXXXX'的字符串转换为对应的unicode字符，即中文显示

  example:
  >>>strMsg = '\u559c\u6b22\u4e00\u4e2a\u4eba'
  >>>print strMsg.decode('unicode-escape')  # ==> '喜欢一个人'
===================================

THANK YOU!
