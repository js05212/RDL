function avgRank = fastLinkRank(C,citationPath,featurePath,trainPath,testPath,thetaPath)
%function avgRank = fastLinkRank(C,citationPath,featurePath,trainPath,testPath,thetaPath)
if nargin<6
    thetaPath = 'final-etap.dat';
end
if nargin<5
    testPath = 'test_ind.dat';
end
if nargin<4
    trainPath = 'train_ind.dat';
end
if nargin<3
    featurePath = 'final-W.dat';
end
if nargin<2
    citationPath = 'citations.dat';
end
if nargin<1
    C = 1;
end
% Load Train,Test Indices, Feature Matrix and Theta
trainind = textread(trainPath);
testind = textread(testPath);
feature = textread(featurePath);
theta = textread(thetaPath);
trainind = trainind+1;
testind = testind+1;
feature = feature(:,1:(size(feature,2)-1));
theta = theta(:,1:(size(theta,2)-1));
% Load Citation and Get the Adjacent Matrix between Training and Testing nodes
f = fopen(citationPath,'r');
edges = sparse(numel(trainind)+numel(testind),numel(trainind)+numel(testind));
tline = fgetl(f);
counter = 1;
while ischar(tline)
    A = strread(tline);
    edges(A(2:end)+1,counter) = 1;
    edges(counter,A(2:end)+1) = 1;
    tline = fgetl(f);
    counter = counter+1;
end
fclose(f);
size(edges);
truth = edges(testind,trainind);

% Get Prediction
pred = zeros(numel(testind),numel(trainind));
testFea = feature(testind,:);
trainFea = feature(trainind,:);

batchsize = 200;
repTrainFea = kron(trainFea,ones(batchsize,1));

for i = 1:floor(numel(testind)/batchsize)
    begin = (i-1)*batchsize+1;
    fea = testFea(begin:(begin+batchsize-1),:);
    pred(begin:(begin+batchsize-1),:) = reshape(predict(repmat(fea,numel(trainind),1).*repTrainFea,theta),[batchsize numel(trainind)]);
end
if floor(numel(testind)/batchsize)*batchsize < numel(testind)
    fea = testFea(floor(numel(testind)/batchsize)*batchsize:end,:);
    repTrainFea = kron(trainFea,ones(size(fea,1),1));
    pred(floor(numel(testind)/batchsize)*batchsize:end,:) = reshape(predict(repmat(fea,numel(trainind),1).*repTrainFea,theta),[size(fea,1) numel(trainind)]);
end
%Compute Link Rank
avgRank = link_rank(pred,truth);

end


function ret = predict(X,theta)

X = [X ones(size(X,1),1)];
% Computes sigmoid

ret = 1.0 ./ (1.0 + exp(-(X*theta')));
end

function [avgRank] = link_rank(pred,truth)
    [~,I] = sort(pred,2,'descend');
    ranking = zeros(size(I));
    for i=1:size(pred,1)
        ranking(i,I(i,:)) = 1:size(truth,2);
    end
    avgRank = nanmean(sum(ranking.*truth,2)./sum(truth,2));
end

