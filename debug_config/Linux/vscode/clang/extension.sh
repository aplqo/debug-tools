mkdir -p ~/.vscode/extensions/llvm-org.lldb-vscode-0.1.0/bin
cp package.json ~/.vscode/extensions/llvm-org.lldb-vscode-0.1.0
cd ~/.vscode/extensions/llvm-org.lldb-vscode-0.1.0/bin
ln -s /usr/lib/llvm-11/bin/lldb-vscode ./lldb-vscode
ln -s /usr/lib/llvm-11/lib/liblldb.so.1 liblldb.so