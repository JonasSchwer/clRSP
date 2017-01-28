% loglogTriangle  draws a triangle with a given slope into a loglog plot
%
%  loglogTriangle(x1,x2,y1,slope,type)
%
%  
%
%  (c) epsBEM team 2012

function loglogTriangle(x1,x2,y1,slope,type)
hold on
slope=-slope;
scale=y1/(x1^slope);
loglog([x1 x2],[x1 x2].^slope*scale,'k');
if type=='l'
  loglog([x1 x2],[x2 x2].^slope*scale,'k');
  loglog([x1 x1],[x1 x2].^slope*scale,'k');
  text(sqrt(x1*x2),(x1^(.1)*x2^(.91))^slope*scale,'1'); 
  text(x1^(.87)*x2^(.05),sqrt(x1*x2)^slope*scale,rats(-slope));
  
elseif type=='u'
  loglog([x1 x2],[x1 x1].^slope*scale,'k');
  loglog([x2 x2],[x1 x2].^slope*scale,'k');
  text(sqrt(x1*x2),(x1^(1)*x2^(.05))^slope*scale,'1');
  text(x1^(.09)*x2^(.865),sqrt(x1*x2)^slope*scale,rats(-slope));
end