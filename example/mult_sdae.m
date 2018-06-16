function mult_sdae
my.from = 265;
my.lv = 100;
my.lu = 0.001;
my.ln = 1e1;
my.noise.drop = 0.3;
my.sparsity.cost = 0.1;
my.pretrain = 'sdae_mult727'; % pretrain
rand('seed',11112);
my.save = 'sdae_mult786';
my.folder = 49;
my.weight_decay = 1e-4; % default is 1e-4
my.dropout = 0.1;
my.iter = 1;
do_pretrain = 1;
gpuDevice(2);

my.sdae.n_epoch = 1000;
my.dae.n_epoch = 1000;
my.minibatch_size = 128;
my.a = 1;
my.b = 0.01;
my.ctrgpu = 1;
my.max_iter = 100;
my.graph = 'S-rand000a.dat';
my.ctr_log = 'ctr.log';
my.adagrad.use = 0;
my.early_stop_thre = 2;
% add the path of RBM code
addpath('..');
% addpath('~/work/Algorithms/liblinear-1.7/matlab');

mkdir(my.save);

% added by hog
mystopping = [-2];

% load content
load 'mult_nor.mat';
%load('sim-data/u10mult.mat');
% load ratings
load(sprintf('ctr-data/folder%d/rmat.mat',my.folder));
%load('sim-data/u10rmat.mat');

% shuffle the training data
perm_idx = randperm(size(X,1));

n_all = size(X, 1);
n_pre_train = ceil(n_all * 3 / 4);
n_pre_valid = floor(n_all /4);
n_train = n_all;
n_valid = 0;

X_ori = X;
X_valid = X(perm_idx(n_train+1:end), :);
X = X(perm_idx(1:n_train), :);
X_pre_valid = X(perm_idx(n_pre_train+1:end), :);
X_pre = X(perm_idx(1:n_pre_train), :);

layers = [size(X,2),1000,200,50];
blayers = [1,1,1,1];
n_layers = length(layers);

use_tanh = 0;

if my.pretrain
    load(my.pretrain);
    fprintf('using pretrain result: %s\n',my.pretrain);
end

if do_pretrain && ~isstr(my.pretrain)
    fprintf('doing pretrain from scratch\n');
    my.fid = fopen(sprintf('%s.log',my.save),'a');
    fprintf(my.fid, 'doing pretrain from scratch\n');
    fclose(my.fid);

    Ds = cell(n_layers - 1, 1);
    H = X_pre;
    H_valid = X_pre_valid;

    for l = 1:n_layers-1
        % construct DAE and use default configurations
        D = default_dae (layers(l), layers(l+1));

        D.data.binary = blayers(l);
        D.hidden.binary = blayers(l+1);

        if use_tanh 
            if l > 1
                D.visible.use_tanh = 1;
            end
            D.hidden.use_tanh = 1;
        else
            if D.data.binary
                mH = mean(H, 1)';
                D.vbias = min(max(log(mH./(1 - mH)), -4), 4);
            else
                D.vbias = mean(H, 1)';
            end
        end

        D.learning.lrate = 1e-1;
        D.learning.lrate0 = 5000;
        D.learning.weight_decay = 0.0001;
        D.learning.minibatch_sz = my.minibatch_size;

        D.valid_min_epochs = 10;

        %D.noise.drop = 0.2; deleted by hog
        D.noise.drop = my.noise.drop;   % added by hog
        D.sparsity.cost = my.sparsity.cost;        % added by hog
        D.noise.level = 0;

        %D.adagrad.use = 1;
        %D.adagrad.epsilon = 1e-8;
        D.adagrad.use = my.adagrad.use;
        D.adadelta.use = 1;
        D.adadelta.epsilon = 1e-8;
        D.adadelta.momentum = 0.99;

        D.iteration.n_epochs = my.dae.n_epoch;

        % save the intermediate data after every epoch
        D.hook.per_epoch = {@save_intermediate, {sprintf('dae_mult_%d.mat', l)}};

        % print learining process
        D.verbose = 0;
        % display the progress
        D.debug.do_display = 0;

        % train RBM
        my.fid = fopen(sprintf('%s.log',my.save),'a');
        fprintf(my.fid, 'Training DAE (%d)\n', l);
        fclose(my.fid);
        fprintf(1, 'Training DAE (%d)\n', l);

        tic;
        D = dae (my, D, H,H_valid,0.1);
        mystopping = [mystopping D.mystopping];

        my.fid = fopen(sprintf('%s.log',my.save),'a');
        fprintf(1, 'Training is done after %f seconds\n', toc);
        fprintf(my.fid, 'Training is done after %f seconds\n', toc);
        fclose(my.fid);

        H = dae_get_hidden(H, D);
        H_valid = dae_get_hidden(H_valid, D);

        Ds{l} = D;
    end % end of for nlayers
end

S = default_sdae (layers);
S.mystopping = mystopping;

S.data.binary = blayers(1);
S.bottleneck.binary = blayers(end);
S.hidden.use_tanh = use_tanh;

S.hook.per_epoch = {@save_intermediate, {my.save}};

S.learning.lrate = 1e-1;
S.learning.lrate0 = 5000;
%S.learning.momentum = 0.9;
S.learning.weight_decay = my.weight_decay;
S.learning.minibatch_sz = my.minibatch_size;

%S.noise.drop = 0.2;
%S.noise.level = 0;
S.noise.drop = my.noise.drop;   % added by hog
S.sparsity.cost = 0.1;          % added by hog
S.adadelta.use = 1;
S.adadelta.epsilon = 1e-8;
S.adadelta.momentum = 0.99;

%S.adagrad.use = 1;
%S.adagrad.epsilon = 1e-8;
S.valid_min_epochs = 10;

S.iteration.n_epochs = my.sdae.n_epoch;

if do_pretrain && my.from==1
    for l = 1:n_layers-1
        S.biases{l+1} = Ds{l}.hbias;
        S.W{l} = Ds{l}.W;
    end
elseif my.from==1
    fprintf('initialize sdae using mean\n');
    my.fid = fopen(sprintf('%s.log',my.save),'a');
    fprintf(my.fid, 'initialize sdae using mean\n');
    fclose(my.fid);
    if S.data.binary
        mH = mean(X, 1)';
        S.biases{1} = min(max(log(mH./(1 - mH)), -4), 4);
    else
        S.biases{1} = mean(X, 1)';
    end
end

my.fid = fopen(sprintf('%s.log',my.save),'a');
fprintf(my.fid, 'Training sDAE\n');
fclose(my.fid);

fprintf(1, 'Training sDAE\n');
tic;
S = sdae (S, X, X_valid, X_ori, 0.1,my,perm_idx);

my.fid = fopen(sprintf('%s.log',my.save),'a');
fprintf(my.fid, 'Training is done after %f seconds\n', toc);
fclose(my.fid);

fprintf(1, 'Training is done after %f seconds\n', toc);

X = X_ori;
H = sdae_get_hidden (my,0,X, S);
H_valid = sdae_get_hidden(my,0,X_valid,S);
save(my.save,'H','S','Ds');




