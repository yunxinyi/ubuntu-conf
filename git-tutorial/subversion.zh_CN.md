#svn - Beta

##什么是Subversion?

Subversion 是一个版本控制系统(SCM),一个软件开发者使用的来支持合作一个团队合作开发，并且实时跟踪软件代码变化。

Subversion被开发者使用，和那些需要软件最新更改的高级用户(在发行之前)。软件用户一般不需要Subversion；他们一般典型的下载由工程编译的官方发行版。

开发者应当读一下【Version Control with Subversion】来使得自己熟悉Subversion。

##现代SCM设施

这里有个漂亮的增记解释了为什么你需要考虑使用分布版本控制系统，和一个主流的DVCSs的比较：
http://www.infoq.com/articles/dvcs-guide

##特性

SourceForge.net 在它的Subversion中提供了以下特性:

* 所有Subversion 1.6.x 的特性均支持。
* 通过svn+ssh和https给开发者提供(读-写)权限。
* 通过svn+ssh，https和http给匿名用户提供(只读)权限。
* 多个Subversion客户端支持，包括:
	
	* TortoiseSVN (MS Windows).
	* 官方SVN客户端(MS Windows, Mac OS X, Linux, BSD).
	
* 源可以通过使用Allura 代码浏览器用web浏览器浏览。
* 源可以通过项目管理员授权。
* 服务使用不受quotas限制。

##管理

###创建一个源

Subversion 服务可以在一个项目创建之时选择安装。也可以如下安装到一个已经存在的项目:

以项目管理员登入，在导航条点击管理图标

* 点击 "Tools".
* 点击 "SVN".
* 选择一个标签名字 (这将决定工程导航中链接的标题)
* 选择挂载点 (这将会影响你的源的URL)

新源被使用推荐的 branch/tags/trunk 格式创建。

###操作

从工具页面，在安装的源下边以下操作可用:
可浏览文件

这里有一个指定的空白分割的文件名扩展列表可以被浏览器浏览(也就是说，纯文本文件)

###刷新源

当有新的更改时，代码浏览器典型的会自动刷新，但是如果需要权限的话也可以手动刷新。

为源指定权限可以在这里配置。Fine-grained 权限控制是不被支持的(例如 根据源内部指定路径限制权限), 如果需要那样的权限控制, 考虑使用不同权限创建多个源。

###删除

这将从服务器上删除源，记住这一般将非常快，偶然会有一个延时。

###Checkout URL

按照默认代码浏览器将包含一条命令供用户checkout源。如果另有制定的默认checkout位置，这个可以在此处指定。

###开发者权限(读-写)

更改源的标准方式是使用一个Subversion客户端

####通过svn+ssh 读/写 访问

svn+ssh将提供比https更快的性能。这个应当在可用的时候尽量使用。

为了通过svn+ssh访问Subversion源，如下配置你的Subversion客户端(用你项目的UNIX组名替换PROJECTNAME， USERNAME用你的用户名):

Hostname: svn.code.sf.net
Port: 22
Protocol: SVN + SSH
Repository Path: /p/PROJECTNAME/code

For clients that use a URL string:

svn+ssh://USERNAME@svn.code.sf.net/p/PROJECTNAME/MOUNTPOINT/

####通过https读/写访问

通过https访问将不如svn+ssh，所以只是用在用svn+ssh访问有问题的时候(例如 如果ssh端口22被阻止)

为了通过https访问Subversion源，如下配置你的Subversion客户端(用你项目的UNIX组名替换PROJECTNAME):

Hostname: svn.code.sf.net
Port: 443
Protocol: HTTPS
Repository Path: /p/PROJECTNAME/MOUNTPOINT

For clients that use a URL string:

https://svn.code.sf.net/p/PROJECTNAME/MOUNTPOINT/

####匿名访问(只读)

以上详细介绍的读/写协议可以用于只读访问。进一步，你可以使用相同的URLs来使用svn和http协议。

例如:

svn://svn.code.sf.net/p/PROJECTNAME/MOUNTPOINT/
http://svn.code.sf.net/p/PROJECTNAME/MOUNTPOINT/

####授权

进行读取操作是无需用户名和密码。

当进行写操作时，你将被提示提供你的your SourceForge.net 用户名和密码。 进行写操作，你的项目管理员必须给你写权限。

####服务器证书无效错误

Suversion用户通过https访问源的时候偶然会发生错误预示着SSL证书issuer不是被信任的，给你一个接受证书的操作:

```
Error validating server certificate for 'https://svn.code.sf.net:443'
- The certificate is not issued by a trusted authority. Use the fingerprint to validate the certificate manually!
Certificate information:
- Hostname: *.svn.code.sf.net
- Valid: from Tue, 09 Oct 2007 13:15:07 GMT until Mon, 08 Dec 2008 14:15:07 GMT
- Issuer: Equifax Secure Certificate Authority, Equifax, US
- Fingerprint: fb:75:6c:40:58:ae:21:8c:63:dd:1b:7b:6a:7d:bb:8c:74:36:e7:8a
(R)eject, accept (t)emporarily or accept (p)ermanently? p
```

这个一般出现在我们更换了SSL证书之后你初次使用Subversion操作的时候。

当你接收到这个错误，我们鼓励你通过把自己的checkout URL键入一个信任的浏览器来验证这个服务器是否是正确的服务器(也就是说  https://svn.code.sf.net/p/PROJECTNAME/MOUNTPOINT/).

你接下来应当检测浏览器接受证书。如果接受了，你可以信任服务器就像其他https站点一样，如银行等。

一旦有效验证之后你应当回到Subversion请求并且告诉客户端当地永久储存SSL证书，如此在我们更新证书之前就不需要再次提示。

##备份

SourceForge.net 使用循环备份我们所有服务器并且在服务器发生灾难时候从这些备份恢复。我们鼓励项目拥有自己的git备份并且在组员偶然事故毁坏数据的时候可以使用项目恢复。

一个SVN源的备份可以使用rsync。

示例 (用你项目的UNIX组名替换PROJECTNAME):

$ rsync -av svn.code.sf.net::p/PROJECTNAME/MOUNTPOINT .

