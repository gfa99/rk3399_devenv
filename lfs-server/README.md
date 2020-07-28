> 当前git repo中包含使用git-lfs跟踪的大文件(≥100M),需要配置git-lfs和lfs-server.
1. cd your_bare_git_repo; 创建一个空的git仓库: **git init --bare**
2. 安装配置git-lfs和lfs-server
3. (如需使用lfs-server)对于当前repo, 需要在 .git/config 中增加如下配置
```
[lfs]
	url = "http://USER:PASSWD@172.16.6.6:9999/" #其中 http, USER, PASSWD, 172.16.6.6 9999 字段需要根据 lfs-server 对应的配置进行填充
[lfs "http://USER:PASSWD@172.16.6.6:9999/"]
	locksverify = false
	access = basic
```
> 接上文,
4. 首先向刚新建的git repo中push一个未使用git-lfs跟踪过大文件的分支(如果master满足要求,则最好: git push origin master)
5. 对应剩余的其它分支(无论是否包含用lfs跟踪过大文件),直接执行命令: **git push origin --all (或 git push origin --mirror)** 将其推送到新建的git repo中
6. 到此新建的git repo即可对外开放作为公用的代码仓库


## lfs-server 配置使用
1. 服务端
1) 将 lfs-server 文件整个复制到git repo的.git目录下(如果有多个repo, 则建议将其复制到所有repo的顶层目录)
2) 修改 run-lfs-server.sh 脚本中 **git_repo_path **变量 (如果是多个repo, 则建议直接设置 **server_home** 变量)
3) 根据需要, 自行配置lfs-server的访问地址,监听端口,管理员用户密码等
4) 在 /etc/rc.local 中添加命令: **nohup bash your_lfs_server_abspath/lfs-server/bin/run-lfs-server.sh & **用以配置server开机启动
2. 客户端
1) 安装配置git-lfs
2) 方式一: 在 ~/.gitconfig 中新增如下配置, 之后 git clone ; git checkout
3) 方式二: 先 git init 初始化一个空的仓库, 然后修改 .git/config 之后 git fetch (注: 对于使用git-lfs跟踪过大文件的分支,必须经过首次checkout下载大文件后才能在本地保存使用)
```
~/.gitconfig 中需要添加的配置如下(在地址前配置用户名和密码方便每次git checkout时不用再手动输入用户密码):

[lfs]
	url = "lfs-server-scheme://lfs-admin-user:lfs-admin-passwd@lfs-server-addr:lfs-server-port/"
	# example:
	# url = "http://lfs_admin_user:lfs_admin_pass@172.16.6.6:9999/"
```

```
.git/config 中需要添加的配置如下(在地址前配置用户名和密码方便每次git checkout时不用再手动输入用户密码):

[remote "origin"]
	url = git@git_server_addr:/git_repo_abspath
	fetch = +refs/heads/*:refs/remotes/origin/*
[lfs]
	url = "lfs-server-scheme://lfs-admin-user:lfs-admin-passwd@lfs-server-addr:lfs-server-port/"
[lfs "lfs-server-scheme://lfs-admin-user:lfs-admin-passwd@lfs-server-addr:lfs-server-port/"]
	locksverify = false
	access = basic
```

## git-lfs 安装配置及使用
1.  (仅首次需要)安装git-lfs
    1) download git-lfs from https://github.com/git-lfs/git-lfs/releases
    2) tar -xvf git-lfs-*.tar.gz && sudo ./install
    3) git lfs install (此命令之后, ~/.gitconfig 中将被新增如下内容)
```
[filter "lfs"]
	clean = git-lfs clean -- %f
	smudge = git-lfs smudge -- %f
	process = git-lfs filter-process
	required = true
```
2. 从git中去除(超过大小限制的)大文件的跟踪
    1) git rm --cached your_giant_files
    2) git commit --amend -CHEAD
3. 使用git-lfs重新跟踪以上大文件
    1) git lfs track your_giant_files
    2) git add your_giant_files -f
    3) git commit -m "..."
