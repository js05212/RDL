function m = sampleneg(mpos,rate)
% sample negative samples from positive sparse matrix mpos
% for citeulike-a and 0.001, it takes 76 seconds
num_pos = size(find(mpos~=0),1)/2;
num_node = size(mpos,1);
m = sparse(num_node,num_node);
num_neg = (num_node*(num_node-1)/2-num_pos)*rate;
num_cur = 0;
while num_cur<num_neg
    if mod(num_cur,10000)==0
        num_cur
    end
    sub1 = ceil(rand()*num_node);
    sub2 = ceil(rand()*num_node);
    if mpos(sub1,sub2)==0 && m(sub1,sub2)==0
        m(sub1,sub2) = 1;
        m(sub2,sub1) = 1;
        num_cur = num_cur+1;
    else
        continue;
    end
end
