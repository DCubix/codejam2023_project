monke = {}

function monke:on_create()
    print('The monke has been created.')

    print('Its name is "' .. self.name .. '" and its tag is "' .. self.tag .. '"')

    for k, v in pairs(self.properties) do
        print(k .. ' = ' .. v)
    end

    self.health = 30.41

    for k, v in pairs(self.properties) do
        print(k .. ' = ' .. v)
    end
end

return monke
