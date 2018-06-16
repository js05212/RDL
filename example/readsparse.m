function m = readsparse(file_in)
ma = textread(file_in,'%s','delimiter','\n','whitespace','');
num_node = size(ma,1);
m = sparse(num_node,num_node);
for i = 1:num_node
    segs = regexp(ma{i},' ','split');
    num_link = str2num(segs{1});
    for j = 2:num_link+1
        m(i,str2num(segs{j})+1) = 1; %0 based to 1 based
    end
end
