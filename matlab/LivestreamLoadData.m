%function [data] = LivestreamLoadData(tagType, nPis, startDate, endDate)
function [data, piNames] = LivestreamLoadData(tagType, nPis, startDate, endDate)

data = [];

n=0;
for d=startDate:endDate
    j = month(d);
    k = day(d);
    filename = ['livestream_MaestroLex1_' sprintf('%02d', j) '-' sprintf('%02d', k) '.log'];
    fprintf([filename ': ']);
    if exist(filename, 'file')
        fprintf(' File opened\n');
        fid = fopen(filename);
        tline = fgets(fid);
        while ischar(tline)
            if strcmp(tagType, tline(1:length(tagType)))
                % We've got tagType data!
                n = n + 1;
            end
            tline = fgets(fid);
        end
        fclose(fid);
    else
        fprintf(' No file found\n');
    end
end

fprintf('%i Data points found\n', n);

data.data = -100 * ones(n, nPis);
temp = datetime([],[],[],[],[],[]);
data.datetime.data = repmat(temp, n, 1);

n=1;

%% Format string for each line of text:
%formatSpec = '%s%{HHmmss}D%s%f%s%f%s%f%s%f%s%f%s%f%s%f%s%f%s%f%s%f%[^\n\r]';
formatSpec = repmat('%s%f', 1, nPis);
formatSpec = ['%s%{HHmmss}D' formatSpec '%[^\n\r]'];

for d=startDate:endDate
    j = month(d);
    k = day(d);
    filename = ['livestream_MaestroLex1_' sprintf('%02d', j) '-' sprintf('%02d', k) '.log'];
    %% Initialize variables.
    delimiter = ',';
    %if nargin<=2
        startRow = 1;
        endRow = inf;
    %end



    %% Open the text file.
    fprintf([filename ': ']);
    if ~exist(filename, 'file')
        fprintf(' No file found\n');
    else
        fprintf(' File opened\n');
        
        fileID = fopen(filename,'r');

        %% Read columns of data according to format string.
        % This call is based on the structure of the file used to generate this
        % code. If an error occurs for a different file, try regenerating the code
        % from the Import Tool.
        dataArray = textscan(fileID, formatSpec, endRow(1)-startRow(1)+1, 'Delimiter', delimiter, 'EmptyValue' ,NaN,'HeaderLines', startRow(1)-1, 'ReturnOnError', false);
        for block=2:length(startRow)
            frewind(fileID);
            dataArrayBlock = textscan(fileID, formatSpec, endRow(block)-startRow(block)+1, 'Delimiter', delimiter, 'EmptyValue' ,NaN,'HeaderLines', startRow(block)-1, 'ReturnOnError', false);
            for col=1:length(dataArray)
                dataArray{col} = [dataArray{col};dataArrayBlock{col}];
            end
        end

        %% Close the text file.
        fclose(fileID);

        %% Post processing for unimportable data.
        % No unimportable data rules were applied during the import, so no post
        % processing code is included. To generate code which works for
        % unimportable data, select unimportable cells in a file and regenerate the
        % script.

        %% Allocate imported array to column variable names
        thisTagRows = strcmp(tagType, dataArray{1});
        for c=1:length(dataArray)
            dataArray{c}(~thisTagRows) = [];
        end

        dayShift = caldiff([dataArray{2}(1), datetime(2016, j, k)],'days');
        nRows = size(dataArray{2}, 1);
        data.datetime.data(n:n+nRows-1) = repmat(dayShift, nRows, 1) + dataArray{2};
        %data.data(n:n+nRows-1) = zeros(size(dataArray{2}, 1), nPis);
        for p = 1:nPis
            data.data((n:n+nRows-1),p) = dataArray{p*2+2};
        end
        n = n + nRows;
        
        piNames = {};
        for a = 3:2:length(dataArray)-1
            piNames = cat(1, piNames, dataArray{a}(1));
        end

    end
end

