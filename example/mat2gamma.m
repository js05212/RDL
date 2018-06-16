function mat2gamma(H,outfile,type)
fid = fopen(outfile,'w');
if type==0
    dlmwrite(outfile,H,'delimiter',' ');
elseif type==1
    H = dae_get_hidden(X,Ds{1});
    [n_doc,n_factor] = size(H);
    for i = 1:n_doc
        fprintf(fid,'%1.6f',H(i,1));
        for j = 2:n_factor
            fprintf(fid,' %1.6f',H(i,j));
        end
        fprintf(fid,'\n');
    end
elseif type==2
    H = nn.W{1}*X'+repmat(nn.b{1},1,size(X,1));
    H(H<0) = 0;
    H = H';
    [n_doc,n_factor] = size(H);
    for i = 1:n_doc
        fprintf(fid,'%1.6f',H(i,1));
        for j = 2:n_factor
            fprintf(fid,' %1.6f',H(i,j));
        end
        fprintf(fid,'\n');
    end
end
fclose(fid);
