function [fileinfo]=read_seg2()
[fileinfo.filename, pathname, filterindex]=uigetfile({'*.DAT';'*.DAT';'*.*'},'File Seg2 Read');
if filterindex
    segFile=fullfile(pathname,fileinfo.filename);
    fid=fopen(segFile,'r','l');
    %% 读取描述块
    describe=fread(fid,4, 'uint16');
    %1-2 3A55H 炮头标识
    %3-4 版本号
    %5-6 道头指针尺寸
    %7-8 道数
    fileinfo.daoshu=describe(4);
    %% 读取道指针地址
    fseek(fid,32,'bof');
    Y1=fread(fid,fileinfo.daoshu,'uint');
    %% 读取道描述块
    for i=1:fileinfo.daoshu
        fseek(fid,Y1(i),'bof');
        data1=fread(fid,2,'uint16');
        data2=fread(fid,2,'uint');
        format=fread(fid,1,'uint16');
        %1-2 道头标识 4422H
        %3-4 道头尺寸
        %5-8 数据段长度
        %9-12 样点数/道
        %13 数据格式
        Y2(i)=data1(2);
        fileinfo.yangdianshu=data2(2);
    end
    %% 读取两道信息
    fseek(fid,Y1(1)+32,'bof');
    Y3(1,:)=fread(fid,Y2(1)-32,'char*1')';
    Y3_1=char(Y3(1,:));
    fseek(fid,Y1(2)+32,'bof');
    Y3(2,:)=fread(fid,Y2(2)-32,'char*1')';
    Y3_2=char(Y3(2,:));
    %% 读取采样率
    goal_arr_char1='SAMPLE_INTERVAL';
    index_star1=strfind(Y3_1,goal_arr_char1);
    index_stop1=index_star1+length(goal_arr_char1)-1;
    fseek(fid,Y1(1)+32+index_stop1,'bof');
    dt=fread(fid,10,'char*1')';
    dt=char(dt);
    fileinfo.dt=str2num(dt);
    %% 读取道间距
    goal_arr_char2='RECEIVER_LOCATION';
    index_star2_1=strfind(Y3_1,goal_arr_char2);
    index_stop2_1=index_star2_1+length(goal_arr_char2)-1;
    fseek(fid,Y1(1)+32+index_stop2_1,'bof');
    x_1=fread(fid,10,'char*1')';
    x_1=char(x_1);
    x_1=str2num(x_1);
    index_star2_2=strfind(Y3_2,goal_arr_char2);
    index_stop2_2=index_star2_2+length(goal_arr_char2)-1;
    fseek(fid,Y1(2)+32+index_stop2_2,'bof');
    x_2=fread(fid,10,'char*1')';
    x_2=char(x_2);
    x_2=str2num(x_2);
    dx=x_2-x_1;
    fileinfo.daojianju=dx;
    fileinfo.daoqishi=x_1;
    if ~(x_2-x_1)
        prompt = {'输入道间距：','输入道起始位置：'};
        dlg_title = '道间距输入';
        num_lines = 1;
        def = {'2.0000','0.0000'};
        data=inputdlg(prompt,dlg_title,num_lines,def);
        fileinfo.daojianju=str2num(data{1});
        fileinfo.daoqishi=str2num(data{2});
    end
    %% 读取每道数据
    for i=1:fileinfo.daoshu
        fseek(fid,Y1(i)+Y2(i),'bof');
        tracedata=fread(fid,fileinfo.yangdianshu,'int32');%读每道数据
        vz(:,i)=tracedata;
    end
    fclose(fid);
    wigb (vz);
else
    warndlg('请选择文件！','警告')
end
%%  作地震图
function wigb (a,scal,x,z,sty,col,amx)
%WIGB: Plot seismic data using wiggles,
%   2008,12,24
%  WIGB(a,scal,x,z,sty,amx)
%
%  IN    a: seismic data
%        scale: multiple data by scale
%        x: x-axis;
%        z: vertical axis (time or depth)
%	 x and z are vectors with offset and time.
%
%	 If only 'a' is enter, 'scal,x,z,amn,amx' are decided automatically;
%	 otherwise, 'scal' is a scalar; 'x, z' are vectors for annotation in
%	 offset and time, 如果sty=1 为wiggle图，但中间有到白线，如果底是彩色的就会看出来，
%    建议如果底是白色的用此选项；sty=2 为变面积，同样中间有白线;sty=3 为wiggle图，
%    但中间没有白线，适合底板颜色是彩色的；sty=4 为变面积，但中间没有白线，适合底板颜色是彩色的；，
%     col:1 is black;2 is red;3 is blue;4 is green;5 is white;amx are the amplitude range.
%
% Author:
% 	Xingong Li, Dec. 1995
% Changes:
%   Dec34,2008: change zeors line fillcolor to black(old is white)
%	Jun11,1997: add amx
% 	May16,1997: updated for v5 - add 'zeros line' to background color
% 	May17,1996: if scal ==0, plot without scaling
% 	Aug6, 1996: if max(tr)==0, plot a line


if nargin == 0, nx=10;nz=10; a = rand(nz,nx)-0.5; end;

[nz,nx]=size(a);

trmx= max(abs(a));
if (nargin <= 6); amx=mean(trmx);  end;
if (nargin <= 5); col=1;  end;
if (nargin <= 4); sty=1;  end;
if (nargin <= 2); x=[1:nx]; z=[1:nz]; end;
if (nargin <= 1); scal =1; end;

if nx <= 1; disp(' ERR:PlotWig: nx has to be more than 1');return;end;

% take the average as dx
dx1 = abs(x(2:nx)-x(1:nx-1));
dx = median(dx1);

dz=z(2)-z(1);
xmx=max(max(a)); xmn=min(min(a));

if scal == 0; scal=1; end;
a = a * dx /amx;
a = a * scal;

fprintf(' PlotWig: data range [%f, %f], plotted max %f \n',xmn,xmx,amx);

% set display range
x1=min(x)-2.0*dx; x2=max(x)+2.0*dx;
z1=min(z)-dz; z2=max(z)+dz;

set(gca,'NextPlot','add','Box','on', ...
    'XLim', [x1 x2], ...
    'YDir','reverse', ...
    'YLim',[z1 z2],...
    'xAxisLocation','top');

switch col
    case 1
        fillcolor = [0 0 0];
    case 2
        fillcolor = [1 0 0];
    case 3
        fillcolor = [0 0 1];
    case 4
        fillcolor = [0 1 0];
    case 5
        fillcolor = [1 1 1];
        
end
%     if col==1
% 	  fillcolor = [0 0 0];
%     elseif col==2
%        fillcolor = [1 0 0];
%     end
linecolor = [0 0 0];
linewidth = 0.1;

z=z'; 	% input as row vector
zstart=z(1);
zend  =z(nz);

for i=nx:-1:1,
    
    tr=a(:,i); 	% --- one scale for all section
    s = sign(tr) ;
    i1= find( s(1:nz-1) ~= s(2:nz) );	% zero crossing points
    npos = length(i1);
    
    
    %12/7/97
    zadd = i1 + tr(i1) ./ (tr(i1) - tr(i1+1)); %locations with 0 amplitudes
    aadd = zeros(size(zadd));
    
    [zpos,vpos] = find(tr >0);
    [zz,iz] = sort([zpos; zadd]); 	% indices of zero point plus positives
    aa = [tr(zpos); aadd];
    aa = aa(iz);
    
    % be careful at the ends
    if tr(1)>0, 	a0=0; z0=1.00;
    else, 		a0=0; z0=zadd(1);
    end;
    if tr(nz)>0, 	a1=0; z1=nz;
    else, 		a1=0; z1=max(zadd);
    end;
    
    zz = [z0; zz; z1; z0];
    aa = [a0; aa; a1; a0];
    
    
    zzz = zstart + zz*dz -dz;
    if (sty==1 || sty==2 )
        %   if (sty==1 || sty==2 &x(i)<100)
        patch( aa+x(i) , zzz,  fillcolor);
        line( 'Color',[1 1 1],'EraseMode','background',  ...
            'Xdata', x(i)+[0 0], 'Ydata',[zstart zend]); % remove zero line
    elseif (sty==3 || sty==4)
        %       elseif (sty==3 || sty==2&x(i)>=100)
        nzero=find(aa==0);
        nlen=length(nzero);
        for ii=1:nlen-1
            if((nzero(ii+1)-nzero(ii))>1)
                patch( aa(nzero(ii):nzero(ii+1),1)+x(i) , zzz(nzero(ii):nzero(ii+1),1),  fillcolor);
            end
        end
    end
    
end
if (sty==1|| sty==3)
    for i=1:nx,
        if trmx(i) ~= 0;    % skip the zero traces
            tr=a(:,i); 	% --- one scale for all section
            line( 'Color',linecolor,'EraseMode','background',  ...
                'LineWidth',linewidth, ...
                'Xdata', tr+x(i), 'Ydata',z);	% negatives line
        elseif (sty==2 || sty==4) % zeros trace
            line( 'Color',linecolor,'EraseMode','background',  ...
                'LineWidth',linewidth, ...
                'Xdata', [x(i) x(i)], 'Ydata',[zstart zend]);
        end;
    end
end