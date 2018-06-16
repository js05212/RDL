function callinkrank(from,to)
if nargin<2
    list = from:from;
end
list = from:to;
num = size(list,2);
for i = 1:num
    fcite = sprintf('./rdl%d/S-cite16000.dat',list(i));
    fw = sprintf('./rdl%d/final-W.dat',list(i));
    ftr = sprintf('./rdl%d/train_ind.dat',list(i));
    fte = sprintf('./rdl%d/test_ind.dat',list(i));
    feta = sprintf('./rdl%d/final-etap.dat',list(i));
    myr = fastLinkRank(1,fcite,fw,ftr,fte,feta);
    fprintf('link rank for rdl%d: %.2f\n',list(i),full(myr));
end
