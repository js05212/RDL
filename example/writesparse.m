function writesparse(m,f_out)
fid = fopen(f_out,'w');
num_node = size(m,1);
for i = 1:num_node
    list = find(m(i,:)~=0)-1;
    num_link = size(list,2);
    fprintf(fid,'%d',num_link);
    for j = 1:num_link
        fprintf(fid,' %d',list(j));
    end
    fprintf(fid,'\n');
end
fclose(fid);
