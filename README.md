# 一个简单的迭代式Web服务器
### 主进程
```
graph TB 
A(获得端口号)-->B
B(创建监听套接字)-->C
C(监听直到建立一个与客户端的连接套接字)-->D
D(执行连接请求)-->E
E(关闭连接套接字)-->C
```

### 执行连接请求
```
graph TB 
A(读取请求行和请求报文)-->B
B(解析URI)-->G
G(常见错误处理)-->C
C(静态还是动态服务)-->D(静态服务)
C-->E(动态服务)
D-->F(终止)
E-->F
```

blog地址：http://codefarmer.me/?p=129