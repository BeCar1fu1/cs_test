# git

## 基础指令

git在本地主要分成这么几个部分：

- workspace工作区：也就是本地工作可以进行修改增删的地方，但是修改完增删的内容不会被暂存或者被git跟踪
- index暂存区：将本地工作区的内容通过`git add`可以添加到暂存区，这样的话之前修改的或者未跟踪的内容就加到这个，提交仓库之前的缓存区了。
- repository仓库：修改进入到这里就变成了一次提交记录`git commit`

如果在本地创建了文件可以用`git status`来查看工作区中的文件的状态。

![img](G:\CPP\CS_review\assets\1756907442208-654f17cb-1d3a-4f0e-8076-fe2466be4ff4.png)

可以看到新建了一个文件是`untracked`

![img](G:\CPP\CS_review\assets\1756907491609-388d1002-0f29-47c1-a4e1-fcc8fc9edd59.png)

通过`git add .`再查看就已经提交到暂存区了。

### 查看修改的状态（`git status`）

`git status`查看修改的状态（暂存区、工作区）

### 添加工作区到暂存区（`git add`）

添加工作区一个或者多个文件的修改到暂存区

### 提交暂存区到本地仓库（`git commit -m "暂存区"`）

### 查看提交日志（`git log`）

```shell
git log [option]
        --all #显示所有分支
        --pretty=oneline #将提交信息显示为一行
        --abbrev-commit  #是的输出的commitid更简洁
        --graph   # 以图的形式显示
```

### 版本回退（`git reset --hard commitID`）

使用`git log --pretty=oneline --abbrev-commit --all --graph`可以查看版本中的commitid

然后在版本回退。

通过`git reflog`这个指令可以看到已经删除的提交记录。

### `touch .gitignore`

如果想要忽略某些文件不让他被git跟踪的话

## 分支管理

#### 查看分支（`git branch`）

#### 切换分支（`git checkout branchname`）

```
git checkout -b branch_name
```

#### 分支合并（`git merge`）

```
git merge branch_name
```

冲突了之后在merge的时候会产生错误，可以自己手动修改然后冲突的内容之后，就可以继续git add->commit等