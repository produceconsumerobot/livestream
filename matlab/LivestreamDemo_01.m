addpath('C:\pub\schtuffa\Ringly\Google Drive\Ringly Share\T02 Test Results Attachments')
addpath('C:\pub\Cross-platform\matlab\Plotting');
addpath('C:\pub\Cross-platform\matlab\Ant');
addpath('C:\pub\Cross-platform\matlab\Others\Kentoolbox\General');

fileName = 'water_quality_6292014_185359_field_allMeasures_NAD27_KYnorth.kgs';
uiimport(fileName);
quadrangle_names = unique(quadrangle_name);
for j=1:length(quadrangle_names)
    nDataPoints(j) = length(find(strcmp(quadrangle_names{j}, quadrangle_name)));
end
[nDataPoints ind] = sort(nDataPoints, 'ascend');
quadrangle_names = quadrangle_names(ind);

dateNums = zeros(length(collection_date),1);
for j=1:length(collection_date)
    try
        dateNums(j) = datenum(collection_date{j});
    catch
        dateNums(j) = 0;
    end
end

analytes = unique(analyte);
analytes = {...
    'Field Conductivity', ....
    'Field Temperature', ...
    'Field pH', ...
    }
for j=length(quadrangle_names)-20:length(quadrangle_names)
    thisQuadInd = strcmp(quadrangle_names{j}, quadrangle_name);
    nOut = fprintf('[%i] %s - ', nDataPoints(j), quadrangle_names{j});
    for m=nOut:24
        fprintf(' ');
    end
    for k=1:length(analytes)
        fprintf('%s: ',  analytes{k});
        thisAnalyteInd = strcmp(analytes{k}, analyte);
        nOut = fprintf('%i %s-%s, ', length(find(thisAnalyteInd & thisQuadInd)), ...
            datestr(min(dateNums(thisAnalyteInd & thisQuadInd)),'mm-dd-yyyy'), datestr(max(dateNums(thisAnalyteInd & thisQuadInd)),'mm-dd-yyyy'));
        for m=nOut:26
            fprintf(' ');
        end
        
    end
    
    fprintf('\n');
end

clf
thisQuadInd = strcmp(quadrangle_names{end}, quadrangle_name);
for k=1:length(analytes)
    thisAnalyteInd = strcmp(analytes{k}, analyte);
    thisData = result(thisAnalyteInd & thisQuadInd);
    save([analytes{k} '.csv'], '-ascii', 'thisData');
    subplot(length(analytes), 1, k);
    plot(result(thisAnalyteInd & thisQuadInd));
    title([analytes{k} ': ' datestr(min(dateNums(thisAnalyteInd & thisQuadInd)),'mm-dd-yyyy') '-' datestr(max(dateNums(thisAnalyteInd & thisQuadInd)),'mm-dd-yyyy')])
    fprintf('%s: ',  analytes{k});
end
set(gcf, 'name', [quadrangle_names{end} ' KY'])
ReportFigSM(gcf, 'newFigs/', {fileName(1:end-4)});



